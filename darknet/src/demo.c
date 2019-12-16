#include "network.h"
#include "detection_layer.h"
#include "region_layer.h"
#include "cost_layer.h"
#include "utils.h"
#include "parser.h"
#include "box.h"
#include "image.h"
#include "demo.h"
#include <sys/time.h>

//SECTION code is added -->
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#define  FIFO_FROM_YOLO   "/tmp/from_yolo_fifo"
#define  FIFO_TO_YOLO     "/tmp/to_yolo_fifo"
#define  FIFO_FILE        "/tmp/VL53L0X"
#define  BUFF_SIZE   1024

int   target_class_a = 0;
char  buff_b[BUFF_SIZE];
char  buff_a[BUFF_SIZE];

int   fd_from_yolo;
int   fd_to_yolo;
int   from_vl53l0x;
//!SECTION code is added <--

#define DEMO 1

//#ifdef OPENCV

static char **demo_names;
static image **demo_alphabet;
static int demo_classes;

static network *net;
static image buff[3];
static image buff_letter[3];
static int buff_index = 0;
static void * cap;
static float fps = 0;
static float demo_thresh = 0;
static float demo_hier = .5;
static int running = 0;

static int demo_frame = 3;
static int demo_index = 0;
static float **predictions;
static float *avg;
static int demo_done = 0;
static int demo_total = 0;
double demo_time;

detection *get_network_boxes(network *net, int w, int h, float thresh, float hier, int *map, int relative, int *num);

int size_network(network *net) {
	int i;
	int count = 0;
	for (i = 0; i < net->n; ++i) {
		layer l = net->layers[i];
		if (l.type == YOLO || l.type == REGION || l.type == DETECTION) {
			count += l.outputs;
		}
	}
	return count;
}

void remember_network(network *net) {
	int i;
	int count = 0;
	for (i = 0; i < net->n; ++i) {
		layer l = net->layers[i];
		if (l.type == YOLO || l.type == REGION || l.type == DETECTION) {
			memcpy(predictions[demo_index] + count, net->layers[i].output, sizeof(float) * l.outputs);
			count += l.outputs;
		}
	}
}

detection *avg_predictions(network *net, int *nboxes) {
	int i, j;
	int count = 0;
	fill_cpu(demo_total, 0, avg, 1);
	for (j = 0; j < demo_frame; ++j) {
		axpy_cpu(demo_total, 1. / demo_frame, predictions[j], 1, avg, 1);
	}
	for (i = 0; i < net->n; ++i) {
		layer l = net->layers[i];
		if (l.type == YOLO || l.type == REGION || l.type == DETECTION) {
			memcpy(l.output, avg + count, sizeof(float) * l.outputs);
			count += l.outputs;
		}
	}
	detection *dets = get_network_boxes(net, buff[0].w, buff[0].h, demo_thresh, demo_hier, 0, 1, nboxes);
	return dets;
}

void *detect_in_thread(void *ptr) {
	running = 1;
	float nms = .4;

	//code is added -->
	float target_xval = .0f;
	float target_wval = .0f;
	float target_hval = .0f;
	float distance_val = .0f;

	//code is added <--

	layer l = net->layers[net->n - 1];
	float *X = buff_letter[(buff_index + 2) % 3].data;
	network_predict(net, X);
	remember_network(net);
	detection *dets = 0;
	int nboxes = 0;
	dets = avg_predictions(net, &nboxes);

	if (nms > 0) do_nms_obj(dets, nboxes, l.classes, nms);

	printf("\033[2J");
	printf("\033[1;1H");
	printf("\nFPS:%.1f\n", fps);
	printf("Objects:\n\n");
	image display = buff[(buff_index + 2) % 3];
	//SECTION code is added -->
	if (draw_detections(display, dets, nboxes, demo_thresh, demo_names, demo_alphabet, demo_classes, target_class_a, &target_xval, &target_wval, &target_hval)) {
		distance_val = target_wval * target_hval;
		printf("[demo.c] target class(%d), xval = %f, wval = %f, hval = %f distance_val = %f \n", target_class_a, target_xval, target_wval, target_hval, distance_val);

		if (target_xval > 0.55) {
			buff_a[0] = 'b';
			write(fd_from_yolo, buff_a, 1);
			printf("%c\n", buff_a[0]);
		}
		else if (target_xval < 0.45) {
			buff_a[0] = 'a';
			write(fd_from_yolo, buff_a, 1);
			printf("%c\n", buff_a[0]);
		}
		else {
			read(from_vl53l0x, buff_b, BUFF_SIZE);
			if (buff_b[0] == '0') {
				buff_a[0] = 'd';
				write(fd_from_yolo, buff_a, 1);
				printf("%c\n", buff_a[0]);
			}
			else if (buff_b[0] == '1') {
				buff_a[0] = 'd';
				write(fd_from_yolo, buff_a, 1);
				printf("%c\n", buff_a[0]);
			}
			else if (buff_b[0] == '2') {
				buff_a[0] = 'i';
				write(fd_from_yolo, buff_a, 1);
				printf("%c\n", buff_a[0]);
			}
			else if (buff_b[0] == '3') {
				buff_a[0] = 'i';
				write(fd_from_yolo, buff_a, 1);
				printf("%c\n", buff_a[0]);
			}
			else if (buff_b[0] == '4') {
				buff_a[0] = 'i';
				write(fd_from_yolo, buff_a, 1);
				printf("%c\n", buff_a[0]);
			}
			else if (buff_b[0] == '5') {
				buff_a[0] = 'c';
				write(fd_from_yolo, buff_a, 1);
				printf("%c\n", buff_a[0]);
			}
			else if (buff_b[0] == '6') {
				buff_a[0] = 'c';
				write(fd_from_yolo, buff_a, 1);
				printf("%c\n", buff_a[0]);
			}
		}

	}
	else/* if(buff_a[0] == 'a' || buff_a[0] == 'b')*/ {
		read(from_vl53l0x, buff_b, BUFF_SIZE);
		if (buff_b[0] == '0') {
			buff_a[0] = 'd';
			write(fd_from_yolo, buff_a, 1);
			printf("%c\n", buff_a[0]);
		}
		else if (buff_b[0] == '1') {
			buff_a[0] = 'd';
			write(fd_from_yolo, buff_a, 1);
			printf("%c\n", buff_a[0]);
		}
		else if (buff_b[0] == '2') {
			buff_a[0] = 'i';
			write(fd_from_yolo, buff_a, 1);
			printf("%c\n", buff_a[0]);
		}
	}
	//!SECTION code is added <--


	free_detections(dets, nboxes);

	demo_index = (demo_index + 1) % demo_frame;
	running = 0;
	return 0;
}

void *fetch_in_thread(void *ptr) {
	free_image(buff[buff_index]);
	buff[buff_index] = get_image_from_stream(cap);
	if (buff[buff_index].data == 0) {
		demo_done = 1;
		return 0;
	}
	letterbox_image_into(buff[buff_index], net->w, net->h, buff_letter[buff_index]);
	return 0;
}

void *display_in_thread(void *ptr) {
	int c = show_image(buff[(buff_index + 1) % 3], "Demo", 1);
	if (c != -1) c = c % 256;
	if (c == 27) {
		demo_done = 1;
		return 0;
	}
	else if (c == 82) {
		demo_thresh += .02;
	}
	else if (c == 84) {
		demo_thresh -= .02;
		if (demo_thresh <= .02) demo_thresh = .02;
	}
	else if (c == 83) {
		demo_hier += .02;
	}
	else if (c == 81) {
		demo_hier -= .02;
		if (demo_hier <= .0) demo_hier = .0;
	}
	return 0;
}

void *display_loop(void *ptr) {
	while (1) {
		display_in_thread(0);
	}
}

void *detect_loop(void *ptr) {
	while (1) {
		detect_in_thread(0);
	}
}


//SECTION code is added -->
void *t_function_a(void *data) {
	int id;
	id = *((int *)data);

	while (1) {
		while (read(fd_to_yolo, buff_a, BUFF_SIZE) != 0) {
			if (buff_a[0] == 'A')                     // apple  39 - bottle
			{
				target_class_a = 39;
				printf("class %d\n", target_class_a);
				buff_a[0] = 0;
			}
			if (buff_a[0] == 'B')                     // banana 0 - person
			{
				target_class_a = 0;
				printf("class %d\n", target_class_a);
				buff_a[0] = 0;
			}
			if (buff_a[0] == 'C')                     // bicycle
			{
				target_class_a = 1;
				printf("class %d\n", target_class_a);
				buff_a[0] = 0;
			}
			if (buff_a[0] == 'D')                     // dog
			{
				target_class_a = 16;
				printf("class %d\n", target_class_a);
				buff_a[0] = 0;
			}
			if (buff_a[0] == 'E')                     // truck
			{
				target_class_a = 7;
				printf("class %d\n", target_class_a);
				buff_a[0] = 0;
			}
		}
	}
}
//!SECTION code is added <--



void demo(char *cfgfile, char *weightfile, float thresh, int cam_index, const char *filename, char **names, int classes, int delay, char *prefix, int avg_frames, float hier, int w, int h, int frames, int fullscreen) {
	//demo_frame = avg_frames;
	image **alphabet = load_alphabet();
	demo_names = names;
	demo_alphabet = alphabet;
	demo_classes = classes;
	demo_thresh = thresh;
	demo_hier = hier;

	//SECTION code is added -->
	pthread_t p_thread[2];
	int thr_id;
	int a = 1;

	thr_id = pthread_create(&p_thread[0], NULL, t_function_a, (void *)&a);
	if (thr_id < 0) {
		perror("thread create error : ");
		exit(0);
	}
	// from wifi thread
	if (-1 == (fd_from_yolo = open(FIFO_FROM_YOLO, O_RDWR, O_CREAT))) {
		if (-1 == mkfifo(FIFO_FROM_YOLO, 0666)) {
			perror("mkfifo() run error");
			exit(1);
		}
		if (-1 == (fd_from_yolo = open(FIFO_FROM_YOLO, O_RDWR))) {
			perror("open() error");
			exit(1);
		}
	}
	// to wifi thread
	if (-1 == (fd_to_yolo = open(FIFO_TO_YOLO, O_RDWR))) {
		if (-1 == mkfifo(FIFO_TO_YOLO, 0666)) {
			perror("mkfifo() run error");
			exit(1);
		}
		if (-1 == (fd_to_yolo = open(FIFO_TO_YOLO, O_RDWR))) {
			perror("open() error");
			exit(1);
		}
	}
	// from VL53L0X
	if (-1 == (from_vl53l0x = open(FIFO_FILE, O_RDWR, O_CREAT))) {
		if (-1 == mkfifo(FIFO_FILE, 0666)) {
			perror("mkfifo() run error");
			exit(1);
		}

		if (-1 == (from_vl53l0x = open(FIFO_FILE, O_RDWR))) {
			perror("open() error");
			exit(1);
		}
	}
	//!SECTION code is added <--



	printf("Demo\n");
	net = load_network(cfgfile, weightfile, 0);
	set_batch_network(net, 1);
	pthread_t detect_thread;
	pthread_t fetch_thread;

	srand(2222222);

	int i;
	demo_total = size_network(net);
	predictions = calloc(demo_frame, sizeof(float*));
	for (i = 0; i < demo_frame; ++i) {
		predictions[i] = calloc(demo_total, sizeof(float));
	}
	avg = calloc(demo_total, sizeof(float));

	if (filename) {
		printf("video file: %s\n", filename);
		cap = open_video_stream(filename, 0, 0, 0, 0);
	}
	else {
		cap = open_video_stream(0, cam_index, w, h, frames);
	}

	if (!cap) error("Couldn't connect to webcam.\n");

	buff[0] = get_image_from_stream(cap);
	buff[1] = copy_image(buff[0]);
	buff[2] = copy_image(buff[0]);
	buff_letter[0] = letterbox_image(buff[0], net->w, net->h);
	buff_letter[1] = letterbox_image(buff[0], net->w, net->h);
	buff_letter[2] = letterbox_image(buff[0], net->w, net->h);

	int count = 0;
	if (!prefix) {
		make_window("Demo", 1352, 1013, fullscreen);
	}

	demo_time = what_time_is_it_now();

	while (!demo_done) {
		buff_index = (buff_index + 1) % 3;
		if (pthread_create(&fetch_thread, 0, fetch_in_thread, 0)) error("Thread creation failed");
		if (pthread_create(&detect_thread, 0, detect_in_thread, 0)) error("Thread creation failed");
		if (!prefix) {
			fps = 1. / (what_time_is_it_now() - demo_time);
			demo_time = what_time_is_it_now();
			display_in_thread(0);
		}
		else {
			char name[256];
			sprintf(name, "%s_%08d", prefix, count);
			save_image(buff[(buff_index + 1) % 3], name);
		}
		pthread_join(fetch_thread, 0);
		pthread_join(detect_thread, 0);
		++count;
	}
}