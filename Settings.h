/* GLOBAL */
#define DISPLAY_DEMO true
#define DISPLAY_CONTOUR_ANNOTATIONS true
#define TRACKER_RECORD true
/* INITIALIZATION */
// initial position and size of the cabture window
#define DEFAULT_SIZE cv::Size(100, 200)
#define DEFAULT_POSITION cv::Point(320, 240)
// number of initial background averaging frames
#define INIT_BACK_SUB 150	

/* SEGMENTATION & TRACKING */
#define FRAME_LEARNING_RATE 0.2
// histogram learning rate 
#define HIST_LEARNING_RATE 0.8
// histogram distance threshold, larger distance 
// frames won't be learnt.
#define HIST_MAX_DISTANCE 0.6
// contribution of backprojection used for the 
// segmentation image, the rest is the background
// subtraction proportion
#define BACKPROJ_PROPORTION 0.7


/* ANALYSIS & RECOGNITION */
// threshold for the finger tip distance from 
// center in multiple of radus
#define TIP_RADIUS 1.8 
// threshold for the thumb height difference 
// in multiple of radius
#define TIP_THUMB_HEIGHT_DIFF 1

/* LOGGER */
#define LOGGER_ON true
#define LOGGER_DIR "logs/"


/* DEMO */
// Target is the cube position desired
#define DEMO_TARGET false
