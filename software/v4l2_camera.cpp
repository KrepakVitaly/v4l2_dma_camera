#include <v4l2_camera.h>

int v4l2_fd_dev = 0; //v4l2 device to write
uint8_t* videosendbuf = NULL;

int user_width = XDMA_FRAME_WIDTH;
int user_height = XDMA_FRAME_WIDTH;
int user_linewidth = XDMA_FRAME_WIDTH*2;

//int v4l2_width = 0;
//int v4l2_height = 0;

//int resize_image_buf

/* if user sieze != v4l2 size, we need to add/delete some raws and column to image */
/* */

//int dma_size = real_width * real_height * 2;

size_t framesize = 0;
size_t linewidth = 0;

void open_vpipe(char* video_device, char* xdma_c2h, char* xdma_user, uint16_t exp, uint8_t pattern, uint16_t iso)
{
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

    vid_format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    vid_format.fmt.pix.width = V4L2_FRAME_WIDTH;
    vid_format.fmt.pix.height = V4L2_FRAME_HEIGHT;
    vid_format.fmt.pix.pixelformat = FRAME_FORMAT;
    if (!format_properties(vid_format.fmt.pix.pixelformat,
        vid_format.fmt.pix.width, vid_format.fmt.pix.height,
        &linewidth,
        &framesize)) {
        printf("unable to guess correct settings for format '%d'\n", FRAME_FORMAT);
    }
    vid_format.fmt.pix.sizeimage = framesize;
    vid_format.fmt.pix.bytesperline = linewidth;
    vid_format.fmt.pix.field = V4L2_FIELD_NONE;
    vid_format.fmt.pix.colorspace = V4L2_COLORSPACE_SRGB; //V4L2_COLORSPACE_RAW;

    printf("V4L2-set-0 VIDIOC_S_FMT\r\n");
    print_format(&vid_format);

    ret_code = ioctl(v4l2_fd_dev, VIDIOC_S_FMT, &vid_format);
    assert(ret_code != -1);

    printf("frame: format=%d\tsize=%lu\n", FRAME_FORMAT, framesize);
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

    return;
}

void update_frame()
{
    //printf("Start exposure_frame\r\n");
    exposure_frame();
    //printf("Start get_dma_frame\r\n");
    get_dma_frame(videosendbuf, framesize);
    //printf("Start write v4l2_fd_dev\r\n");
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
        break;
    case V4L2_PIX_FMT_SRGGB12: case V4L2_PIX_FMT_SGRBG12: case V4L2_PIX_FMT_SGBRG12: case V4L2_PIX_FMT_SBGGR12:
        lw = (ROUND_UP_2(width) * 2);
        fw = lw * height;
        break;
    case V4L2_PIX_FMT_GREY:
        lw = (ROUND_UP_2(width) * 2);
        fw = lw * height;
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
    printf("vidsendbuf freed\r\n");
    close(v4l2_fd_dev);
    printf("V4L2 sink closed\r\n");
    deinit_dma_camera();
    return;
}