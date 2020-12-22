#include <v4l2_camera.h>

int v4l2_fd_dev = 0; //v4l2 device to write
uint8_t* videosendbuf = NULL;

int user_width = DEFAULT_FRAME_WIDTH;
int user_height = DEFAULT_FRAME_HEIGHT;

uint8_t need_buf_reorder = 0;
uint8_t* tmp_buf = NULL;

/* if user sieze != v4l2 size, we need to add/delete some raws and column to image */
/* */

size_t framesize = 0;
size_t linewidth = 0;


void open_vpipe(char* video_device, unsigned int width, unsigned int height, char* pixfmt, char* xdma_c2h, char* xdma_user, uint16_t exp, uint8_t pattern, uint16_t iso)
{
    user_width = width;
    user_height = height;

    init_dma_camera(xdma_c2h, xdma_user, exp, pattern, iso);

    int ret_code = 0;

    printf("using output device: %s\r\n", video_device);

    v4l2_fd_dev = open(video_device, O_RDWR); //todo add O_NONBLOCK?
    assert(v4l2_fd_dev >= 0);

    printf("V4L2 sink opened O_RDWR, descriptor %d\r\n", v4l2_fd_dev);
    if (v4l2_fd_dev < 0)
    {
        fprintf(stderr, "Failed to open v4l2sink device. (%s)\n", strerror(errno));
        close_vpipe();
        exit(errno);
    }

    struct v4l2_capability vid_caps;
    printf("V4L2-get VIDIOC_QUERYCAP\r\n");
    ret_code = ioctl(v4l2_fd_dev, VIDIOC_QUERYCAP, &vid_caps);
    assert(ret_code != -1);

    struct v4l2_fmtdesc vid_fmtdesc;
    memset(&vid_fmtdesc, 0, sizeof(vid_fmtdesc));
    vid_fmtdesc.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    printf("V4L2-get VIDIOC_ENUM_FMT\r\n");
    while (ioctl(v4l2_fd_dev, VIDIOC_ENUM_FMT, &vid_fmtdesc) == 0)
    {
        printf("%s\n", vid_fmtdesc.description);
        vid_fmtdesc.index++;
    }

    struct v4l2_format vid_format;
    memset(&vid_format, 0, sizeof(vid_format));
    vid_format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    printf("V4L2-get-0 VIDIOC_G_FMT\r\n");
    ret_code = ioctl(v4l2_fd_dev, VIDIOC_G_FMT, &vid_format);
    if (ret_code < 0)
    {
        int err = errno;
        printf("VIDIOC_G_FMT Errcode %d %d\r\n", ret_code, err);
        //TODO free resources
        exit(EXIT_FAILURE);
    }
    print_format(&vid_format);


    printf("get_pixformat_by_name %d \r\n", get_pixformat_by_name(pixfmt));

    vid_format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    vid_format.fmt.pix.width = user_width;
    vid_format.fmt.pix.height = user_height;
    vid_format.fmt.pix.pixelformat = get_pixformat_by_name(pixfmt);
    if (!format_properties(vid_format.fmt.pix.pixelformat,
        vid_format.fmt.pix.width, vid_format.fmt.pix.height,
        &linewidth,
        &framesize)) {
        printf("unable to guess correct settings for format '%d' %s\n", vid_format.fmt.pix.pixelformat, pixfmt);
    }
    vid_format.fmt.pix.sizeimage = framesize;
    vid_format.fmt.pix.bytesperline = linewidth;
    vid_format.fmt.pix.field = V4L2_FIELD_NONE;
    vid_format.fmt.pix.colorspace = V4L2_COLORSPACE_DEFAULT; //V4L2_COLORSPACE_RAW;

    printf("V4L2-set-0 VIDIOC_S_FMT\r\n");
    print_format(&vid_format);

    ret_code = ioctl(v4l2_fd_dev, VIDIOC_S_FMT, &vid_format);
    assert(ret_code != -1);

    printf("frame: format=%d\tsize=%lu\n", get_pixformat_by_name(pixfmt), framesize);
    print_format(&vid_format);

    memset(&vid_format, 0, sizeof(vid_format));
    vid_format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    printf("V4L2-get-1 VIDIOC_G_FMT\r\n");
    ret_code = ioctl(v4l2_fd_dev, VIDIOC_G_FMT, &vid_format);
    if (ret_code < 0)
    {
        int err = errno;
        printf("VIDIOC_G_FMT Errcode %d %d\r\n", ret_code, err);
        close_vpipe();
        exit(EXIT_FAILURE);
    }
    print_format(&vid_format);

    videosendbuf = (uint8_t*)malloc(sizeof(uint8_t) * framesize);
    
    if (need_buf_reorder == 1)
    {
        tmp_buf = (uint8_t*)malloc(sizeof(uint8_t) * user_width * user_height * 2);
    }
    if (need_buf_reorder == 2)
    {
        tmp_buf = (uint8_t*)malloc(sizeof(uint8_t) * user_width * user_height * 4);
    }
    return;
}

void update_frame()
{
    int xdma_width = user_width;
    int xdma_height = user_height;
    exposure_frame();
    if (need_buf_reorder == 1)
    {

        get_dma_frame(tmp_buf, user_width * user_height * 2);
        reodrder_data_8to_12bit_rggb(tmp_buf, user_width, user_height, videosendbuf, linewidth, framesize / linewidth);
    }
    if (need_buf_reorder == 2)
    {
        get_dma_frame(tmp_buf, user_width * user_height * 4);
        reodrder_data_ir_camera_rggb(tmp_buf, user_width, user_height, videosendbuf, linewidth, framesize / linewidth);
    }
    else
    {
        get_dma_frame(videosendbuf, framesize);
    }
    write(v4l2_fd_dev, videosendbuf, framesize);
    
}


void print_format(struct v4l2_format* vid_format) {
    printf("	vid_format->type                =%d\n", vid_format->type);
    printf("	vid_format->fmt.pix.width       =%d\n", vid_format->fmt.pix.width);
    printf("	vid_format->fmt.pix.height      =%d\n", vid_format->fmt.pix.height);
    printf("	vid_format->fmt.pix.pixelformat =%d\n", vid_format->fmt.pix.pixelformat);
    printf("	vid_format->fmt.pix.sizeimage   =%d\n", vid_format->fmt.pix.sizeimage);
    printf("	vid_format->fmt.pix.field       =%d\n", vid_format->fmt.pix.field);
    printf("	vid_format->fmt.pix.bytesperline=%d\n", vid_format->fmt.pix.bytesperline);
    printf("	vid_format->fmt.pix.colorspace  =%d\n", vid_format->fmt.pix.colorspace);
}


unsigned int get_pixformat_by_name(char* pixfmr_str)
{
    if (!strcmp(pixfmr_str, "V4L2_PIX_FMT_YUV420"))
        return V4L2_PIX_FMT_YUV420;
    if (!strcmp(pixfmr_str, "V4L2_PIX_FMT_YVU420"))
        return V4L2_PIX_FMT_YVU420;
    if (!strcmp(pixfmr_str, "V4L2_PIX_FMT_UYVY"))
        return V4L2_PIX_FMT_UYVY;
    if (!strcmp(pixfmr_str, "V4L2_PIX_FMT_Y41P"))
        return V4L2_PIX_FMT_Y41P;
    if (!strcmp(pixfmr_str, "V4L2_PIX_FMT_YUYV"))
        return V4L2_PIX_FMT_YUYV;
    if (!strcmp(pixfmr_str, "V4L2_PIX_FMT_YVYU"))
        return V4L2_PIX_FMT_YVYU;
    if (!strcmp(pixfmr_str, "V4L2_PIX_FMT_SBGGR8"))
        return V4L2_PIX_FMT_SBGGR8;
    if (!strcmp(pixfmr_str, "V4L2_PIX_FMT_SGBRG8"))
        return V4L2_PIX_FMT_SGBRG8;
    if (!strcmp(pixfmr_str, "V4L2_PIX_FMT_SGRBG8"))
        return V4L2_PIX_FMT_SGRBG8;
    if (!strcmp(pixfmr_str, "V4L2_PIX_FMT_SRGGB8"))
        return V4L2_PIX_FMT_SRGGB8;
    if (!strcmp(pixfmr_str, "V4L2_PIX_FMT_SRGGB12"))
        return V4L2_PIX_FMT_SRGGB12;
    if (!strcmp(pixfmr_str, "V4L2_PIX_FMT_SGRBG12"))
        return V4L2_PIX_FMT_SGRBG12;
    if (!strcmp(pixfmr_str, "V4L2_PIX_FMT_SGBRG12"))
        return V4L2_PIX_FMT_SGBRG12;
    if (!strcmp(pixfmr_str, "V4L2_PIX_FMT_SBGGR12"))
        return V4L2_PIX_FMT_SBGGR12;
    if (!strcmp(pixfmr_str, "V4L2_PIX_FMT_Y16"))
        return V4L2_PIX_FMT_Y16;   
    if (!strcmp(pixfmr_str, "V4L2_PIX_FMT_GREY"))
        return V4L2_PIX_FMT_GREY;

    if (!strcmp(pixfmr_str, "V4L2_PIX_FMT_ABGR32"))
        return V4L2_PIX_FMT_ABGR32;
    if (!strcmp(pixfmr_str, "V4L2_PIX_FMT_ARGB32"))
        return V4L2_PIX_FMT_ARGB32;
    if (!strcmp(pixfmr_str, "V4L2_PIX_FMT_XBGR32"))
        return V4L2_PIX_FMT_XBGR32;
    if (!strcmp(pixfmr_str, "V4L2_PIX_FMT_XRGB32"))
        return V4L2_PIX_FMT_XRGB32;
    return -1;
}

int format_properties(const unsigned int format,
    const unsigned int width,
    const unsigned int height,
    size_t* linewidth,
    size_t* framewidth)
{
    size_t lw, fw;
    switch (format)
    {
    case V4L2_PIX_FMT_YUV420: case V4L2_PIX_FMT_YVU420:
        lw = width; /* ??? */
        fw = ROUND_UP_4(width) * ROUND_UP_2(height);
        fw += 2 * ((ROUND_UP_8(width) / 2) * (ROUND_UP_2(height) / 2));
        break;
    case V4L2_PIX_FMT_UYVY: case V4L2_PIX_FMT_Y41P: case V4L2_PIX_FMT_YUYV: case V4L2_PIX_FMT_YVYU:
        lw = (ROUND_UP_2(width) * 2);
        fw = lw * height;
        break;
    case V4L2_PIX_FMT_SBGGR8: case V4L2_PIX_FMT_SGBRG8: case V4L2_PIX_FMT_SGRBG8: case V4L2_PIX_FMT_SRGGB8:
        lw = ROUND_UP_2(width);
        fw = lw * height;
        //need_buf_reorder = 1;
        printf("Slow mode with middle buffer for reorder bytes from 12 to 8 bit\r\n");
        break;
    case V4L2_PIX_FMT_SRGGB12: case V4L2_PIX_FMT_SGRBG12: case V4L2_PIX_FMT_SGBRG12: case V4L2_PIX_FMT_SBGGR12:
        lw = (ROUND_UP_2(width) * 2);
        fw = lw * height;
        break;
    case V4L2_PIX_FMT_GREY: case V4L2_PIX_FMT_Y16:
        lw = (ROUND_UP_2(width) * 2);
        fw = lw * height;
        need_buf_reorder = 2; // IR camera format
        break;
    case V4L2_PIX_FMT_ABGR32: case V4L2_PIX_FMT_ARGB32: case V4L2_PIX_FMT_XBGR32: case V4L2_PIX_FMT_XRGB32:
        lw = (ROUND_UP_2(width) * 4);
        fw = lw * height;
        need_buf_reorder = 2; // IR camera format
        break;
    default:
        return 0;
    }

    if (linewidth) *linewidth = lw;
    if (framewidth) *framewidth = fw;

    return 1;
}

void close_vpipe()
{
    free(videosendbuf);
    free(tmp_buf);
    printf("video buffers freed\r\n");
    close(v4l2_fd_dev);
    printf("V4L2 sink closed\r\n");
    deinit_dma_camera();
    return;
}