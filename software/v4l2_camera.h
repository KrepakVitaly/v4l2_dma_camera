#define V4L2_FRAME_WIDTH  2048
#define V4L2_FRAME_HEIGHT 1558


#define ROUND_UP_2(num)  (((num)+1)&~1)
#define ROUND_UP_4(num)  (((num)+3)&~3)
#define ROUND_UP_8(num)  (((num)+7)&~7)
#define ROUND_UP_16(num) (((num)+15)&~15)
#define ROUND_UP_32(num) (((num)+31)&~31)
#define ROUND_UP_64(num) (((num)+63)&~63)

#define DEFAULT_VIDEO_DEVICE "/dev/video0"

void print_format(struct v4l2_format* vid_format);

