/// 
//  mandel.c
//  Based on example code found here:
//  https://users.cs.fiu.edu/~cpoellab/teaching/cop4610_fall22/project3.html
//
//  Converted to use jpg instead of BMP and other minor changes
//  
///
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <math.h>
#include <pthread.h>
#include "jpegrw.h"

// local routines
static int iteration_to_color(int i, int max);
static int iterations_at_point(double x, double y, int max);
static void compute_image(imgRawImage *img, double xmin, double xmax,
									double ymin, double ymax, int max, int num_threads);
static void show_help();
void* thread_compute_image(void* arg);

/*
 Threads use this struct to access values from their respective function
 */
typedef struct {
    imgRawImage* img;
    double xmin, xmax, ymin, ymax;
    int max_iterations;
    int start_row;
    int end_row;
    int width;
    int height;
} thread_data;



int main( int argc, char *argv[]) {
	char c;

	// These are the default configuration values used
	// if no command line arguments are given.
	char outfile[14];
	double xcenter = 0;
	double ycenter = 0;
	double xscale_0 = 4;
	double yscale_0 = 0; // calc later
	
	int    image_width = 1000;
	int    image_height = 1000;
	int    max = 1000;
	int    num_proc = 1; // Total number of processes
	int    total_images = 50; // Total number of images
	int    num_threads = 1; // Total number of threads

	// For each command line argument given,
	// override the appropriate configuration value.

	while((c = getopt(argc,argv,"x:y:s:W:H:m:o:n:i:t:h"))!=-1) {
		switch(c) {
			case 'x':
				xcenter = atof(optarg);
				break;
			case 'y':
				ycenter = atof(optarg);
				break;
			case 's':
				xscale_0 = atof(optarg);
				break;
			case 'W':
				image_width = atoi(optarg);
				break;
			case 'H':
				image_height = atoi(optarg);
				break;
			case 'm':
				max = atoi(optarg);
				break;
			case 'o':
				strcpy(outfile, optarg); 
				break;
			case 'n':
				num_proc = atoi(optarg);
				break;
			case 'i':
				total_images = atoi(optarg);
				break;
			case 't':
				num_threads = atoi(optarg);
				if (num_threads > 20 || num_threads < 1) {
					num_threads = 1; // Defaults if invalid int
				}
				break;
			case 'h':
				show_help();
				exit(1);
				break;
		}
	}

	// Calculate y scale based on x scale (settable) and image sizes in X and Y (settable)
	yscale_0 = xscale_0 / image_width * image_height;
	// Display the configuration of the image.
	printf("mandel: x=%lf y=%lf xscale=%lf yscale=%1f max=%d outfile=%s\n",xcenter,ycenter,xscale_0,yscale_0,max,outfile);

	
		printf("XScale: %f\n YScale: %f\n", xscale_0, yscale_0);

        int range_per_proc = total_images / num_proc; // Divide evenly among processes
        int remainder = total_images % num_proc;     // Handle leftover images

        pid_t pids[num_proc];  // Array to hold PIDs of child processes

        for (int i = 0; i < num_proc; i++) {
            pid_t pid = fork(); // Creates child process
			// Store the child's PID within parent immediately, important
			// to avoid race condition between child pid population and
			// parent waitpid array check
                pids[i] = pid;
            if (pid == 0) { // Child process

				// Used a ternary to simplify and condense these lines
                int start = i * range_per_proc + (i < remainder ? i : remainder);  // First remainder processes get 1 extra image
                int end = start + range_per_proc - 1 + (i < remainder ? 1 : 0);    // Adjust for remainder

                // Making sure no process overshoots the total number of images
                if (end >= total_images) {
                    end = total_images - 1;
                }

                // Print which process is handling which range
                printf("Process %d handling range %d to %d\n", i, start, end);

                // Generate images for this range
                for (int j = start; j <= end; j++) {
			
			        // This equation will create picture with exponentially decaying scale
			        // xscale = xscale
			        // scale = scale_0 * e^(-k*(j_norm/totalimages)) exponentially decay function
		    	    // Specifically used this with normalized j so that independent processes
		        	// Can calculate images with scales line up with each other when combined
		        	int k = 15.0; // Variable controlling rate of decay
		        	int j_norm = j + start; // Calculate normalized j
		        	double xscale = xscale_0 * exp(-k * (double)j_norm / total_images);
		        	double yscale = yscale_0 * exp(-k * (double)j_norm / total_images);
                    char outfile[20];

                    snprintf(outfile, sizeof(outfile), "mandel%d.jpg", j);
                    imgRawImage* img = initRawImage(image_width, image_height);
                    setImageCOLOR(img, 0);
                    compute_image(img, xcenter - xscale / 2, xcenter + xscale / 2,
                    ycenter - yscale / 2, ycenter + yscale / 2, max, num_threads);
                    storeJpegImageFile(img, outfile);
                    freeRawImage(img);
					printf("Image %d is finished\n", j);
					fflush(stdout);  // Ensure output is immediately written to the terminal
					// intentional so that each moment image is created, the speed can be easily
					// seen from the terminal and avoid confusion in regard to efficiency
                }
                exit(0); // Child exits after generating images
            } else if (pid < 0) {
                perror("Fork failed");
                return 1;
            }
        }       
        // Parent process waits for all child processes
        for (int i = 0; i < num_proc; i++) {
            waitpid(pids[i], NULL, 0);  // Wait for each child to finish
        }
        printf("All processes have finished.\n");
        return 0;

}




/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/

int iterations_at_point( double x, double y, int max )
{
	double x0 = x;
	double y0 = y;

	int iter = 0;

	while( (x*x + y*y <= 4) && iter < max ) {
		// Decided to change squared calculation into a cubic
		double xt = x*x - y*y + x0; // Real part of z^2 + c
		double yt = 2*x*y + y0;  // Imaginary part of z^2 + c


		x = xt;
		y = yt;

		iter++;
	}

	return iter;
}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/

void compute_image(imgRawImage* img, double xmin, double xmax, double ymin, double ymax, int max, int num_threads) {
	// Certain parts of code changed from original, some is simply kept exactly as it was
    // Creating array of thread ids and data
    pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
	// Pointer to thread data arguments
    thread_data* thread_args = (thread_data*)malloc(num_threads * sizeof(thread_data));

	// Checking to see if either malloc failed
    if (threads == NULL || thread_args == NULL) {
        perror("Error. Memory allocation failed...");
        return;
    }


    int height = img->height;

    // Determining how many rows each thread will get, splitting the image
	// into horizontal slices from top down
    int rows_per_thread = height / num_threads;

    // Create threads and assign each one a portion of the image to process
    for (int i = 0; i < num_threads; i++) {
		// Starting by iterating through the array of thread arguments and assigning each one
		// the respective variables values inside of the thread_data struct
        thread_args[i].img = img;
        thread_args[i].xmin = xmin;
        thread_args[i].xmax = xmax;
        thread_args[i].ymin = ymin;
        thread_args[i].ymax = ymax;
        thread_args[i].max_iterations = max;
        thread_args[i].width = img->width;
        thread_args[i].height = height;

        // Dividing the rows among all of the threads using the value of i and the previous
		// rows_per_thread value that was calculated
        thread_args[i].start_row = i * rows_per_thread;
		// This if else logic handles the remaining rows if there are any
		if (i == num_threads - 1) {
			thread_args[i].end_row = height;
		} else {
			thread_args[i].end_row = (i + 1) * rows_per_thread;
		}

        // Create the thread and pass the thread data to it as a void pointer to access struct values
        if (pthread_create(&threads[i], NULL, thread_compute_image, (void*)&thread_args[i]) != 0) {
            perror("Failed to create thread");
            return;
        }
    }

    // Wait for all threads to finish so program doesnt go ahead
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Free dynamically allocated memory to avoid leaks / check with valgrind..
    free(threads);
    free(thread_args);
}

void* thread_compute_image(void* arg) {
    // Cast the void* to thread_data* so we can access the data
    thread_data* data = (thread_data*)arg;

	// Accessing all of the struct values from the pointer *void arg so that
	// calculations can be carried out within the thread function
    int width = data->width;
    int height = data->height;
    double xmin = data->xmin;
    double xmax = data->xmax;
    double ymin = data->ymin;
    double ymax = data->ymax;
    int max = data->max_iterations;
    imgRawImage* img = data->img;

    // Calculating the rows that are specifically for this thread
	// Goes from the strating row value to the end row value in struct
	// Inner for loop runs through the previously written code
	// j is acting as y value and x is acting as the x value
	// j -> row and i -> column
    for (int j = data->start_row; j < data->end_row; j++) {
        for (int i = 0; i < width; i++) {
			// using previously written code to find x y point
            double x = xmin + i * (xmax - xmin) / width;
            double y = ymin + j * (ymax - ymin) / height;

            // using previously written code to get int iters
            int iters = iterations_at_point(x, y, max);

            // Using previously written code to set the pixel in the bitmap
            setPixelCOLOR(img, i, j, iteration_to_color(iters, max));
        }
    }
	
    return NULL;
}



/*
Convert a iteration number to a color.
Changed this function. Instead of using grey scale, it converts HSV values into RGB
To create a sort of rainbow effect.
*/
int iteration_to_color(int iters, int max) {
    double t = (double)iters / max; // Normalized iteration count (0 to 1)
	// Uses normalized t between 0 and 1  and multiplies by 360 to get angle
    // Using 1440 degrees represents four full cycles for more variation
    double hue = t * 1440.0;         // Map t to hue (0 to 1440 degrees)

    // Convert hue to RGB
	// Hue is an angle on a color wheel
	// Saturation - color intensity and Value - brightness
    double r, g, b;
    int i = (int)(hue / 60.0); // determining which section color fals in R/G/B
    double f = (hue / 60.0) - i; // Fractional position in hue within 60 deg
    double q = 1.0 - f; // color intensity adjustment, based on f and s (sat)

    switch (i % 6) {
        case 0: r = 1.0; g = f; b = 0.0; break;
        case 1: r = q; g = 1.0; b = 0.0; break;
        case 2: r = 0.0; g = 1.0; b = f; break;
        case 3: r = 0.0; g = q; b = 1.0; break;
        case 4: r = f; g = 0.0; b = 1.0; break;
        case 5: r = 1.0; g = 0.0; b = q; break;
    }

    int red = (int)(r * 255);
    int green = (int)(g * 255);
    int blue = (int)(b * 255);
	// return statement combines red green and blue values into a 32bit integer,
	// which is the bit length standard for RGB values
    return (red << 16) | (green << 8) | blue;
}



// Show help message
void show_help()
{
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
	printf("-x <coord>  X coordinate of image center point. (default=0)\n");
	printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
	printf("-s <scale>  Scale of the image in Mandlebrot coordinates (X-axis). (default=4)\n");
	printf("-W <pixels> Width of the image in pixels. (default=1000)\n");
	printf("-H <pixels> Height of the image in pixels. (default=1000)\n");
	printf("-o <file>   Set output file. (default=mandel.bmp)\n");
    printf("-n <proc>   Number of processes to be used in calculations (default=1)\n");
	printf("-i <image>  Number of images to generate. (default=50)\n");
	printf("-t <thread> Number of threads to use during calculations. (default=1 / max=20)\n");
	printf("-h          Show this help text.\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}
