# Jadyn Osborne Lab 11 : 11/25/2024
# System Programming Lab 11 Multiprocessing
## Implementation
Only existing files were edited, all major changes occurred within `mandel.c`.
To implement parallel processing, the parent forks within a for loop according to the number 
of processes provided by the user (default:1).
Each process works independently and creates the images based on each processes'
calculated start and end times. (Certain processes will contain extra remainder calculations).
All `mandel*.jpg` images have numbered file names dependent on what image they are generating
within their given start and end range. Lastly, all files are numbered in correct order, so
compilation of the jpgs using `ffmpeg` will work as it increments through by decimal value.
Threads have been implemented to divide up the processing of individual images. 
Multiple threads process horizontal sections of an image at the same time when called to 
their respective functions containing a struct of necessary data for computation
## Graph of runtime results
X axis: Number of processes
Y axis: Number of threads
Z axis: Time in seconds
Red (7 Processes)
Blue (8 Processes)
Green (9 Processes)
![image](https://github.com/user-attachments/assets/a3c411dd-98bf-4dba-a003-8de451e2067f)
![image](https://github.com/user-attachments/assets/0a73f726-185d-42e1-afaa-445df8c371a1)
![image](https://github.com/user-attachments/assets/ef6f825b-0e01-43c6-aeed-d95859dbcd2b)
Below is a link to the `interactive Desmos graph`:
https://www.desmos.com/3d/gzn7r7btbc
## Result Analysis
Results are logical, around 7,8, and 9 processes is where the program experiences its fastest
speeds for generating specifically 50 images. Including less than 7 processes or more than 9
processes was unnecessary as logically these will either get slower with much more overhead
or much slower with less processing power. I tested 7, 8, and 9 processes each with 8-12
threads each. The sweet spot for processing times occurred with 8 processes and 12 threads.
The time for this was 15.113s. 9 processes had its quickest time with 9 threads at 16.211s and
7 processes had its quickest time with 12 threads at 15.307s. Overall, avoiding major overhead 
with increasing processes while maximizing the thread count for computational power seems to be
the reason that 8 processors and 12 threads is the quickest time I was able to achieve.
