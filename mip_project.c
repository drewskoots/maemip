/*******************************************************************************
* rc_project_template.c
*
* This is meant to be a skeleton program for robotics cape projects. 
* Change this description and file name 
*******************************************************************************/

// usefulincludes is a collection of common system includes for the lazy
// This is not necessary for roboticscape projects but here for convenience
#include <rc_usefulincludes.h> 
// main roboticscape API header
#include <roboticscape.h>


// function declarations
void on_pause_pressed();
void on_pause_released();
int modeflag=0; //0=stay at reference postiion, 1= follow left wheel


void print_usage(){
	printf("\n");
	printf("-f          Enable Wheel Follow Mode\n");
	printf("-k {float}  Define proportional feedback Constant (between 0 and 1)\n");
	printf("-s          Enable Stay at Reference Mode\n");  
	printf("-h          print this help message\n");
	printf("\n");
}


/*******************************************************************************
* int main() 
*
* This template main function contains these critical components
* - call to rc_initialize() at the beginning
* - main while loop that checks for EXITING condition
* - rc_cleanup() at the end
*******************************************************************************/
int main(int argc, char *argv[]){
	
	
	//Define the 'polarity' of motors due to them being mirror images of each other
	int rightpolarity=-1;
	int leftpolarity=1;

	//Define motor and encoder channels for left and right side.
	int rightchannel=2;
	int leftchannel=3;	
	
	float gearboxratio=4*15*35.577; //Define conversion factor of ticks per radian
	float k=0.25; //Define proportional constant 

	
	
	// parse arguments
	opterr = 0;
	int c;
	float in;
	while ((c = getopt(argc, argv, "k:fsh")) != -1){
		switch (c){
		case 'k': // custom proportional response specified
			in = atof(optarg);
			if(in>0 && in<2){
				k = in;
			}
			else{
				printf("k must be between 0 and 2.\n");
				return -1;
			}
			break;
		case 'f': //follow mode option
			modeflag=1;
			break;
		case 's': //stay at ref option
			modeflag=0;
			break;
		case 'h': //print help
			print_usage();
			return -1;
			break;
		default:
			print_usage();
			return -1;
			break;
		}
		printf("%c", c);
	}


	// always initialize cape library first
	if(rc_initialize()){
		fprintf(stderr,"ERROR: failed to initialize rc_initialize(), are you root?\n");
		return -1;
	}


	// do your own initialization here
	rc_set_pause_pressed_func(&on_pause_pressed);
	rc_set_pause_released_func(&on_pause_released);

	//Wake motors out of low power state
	if(rc_enable_motors()){
		fprintf(stderr,"ERROR: failed to enable motors\n");
		return -1;
	}

	// done initializing so set state to RUNNING
	rc_set_state(RUNNING); 
	
	//Print 'skelton' to display wheel positions
	printf("\nEncoder positions (Radians)\n");
	printf("   Right   |");
	printf("   Left   |");
	printf(" \n");


	// Keep looping until state changes to EXITING
	while(rc_get_state()!=EXITING){
		
		//Get raw wheel positions from encoders
		int rightpos=rc_get_encoder_pos(rightchannel);
		int leftpos=rc_get_encoder_pos(leftchannel);
		
		
		//Convert raw positions to radians
		float rightrads=2*3.14159265358979323*rightpos/gearboxratio;
		float leftrads=2*3.14159265358979323*leftpos/gearboxratio;	
		
		//update wheel postion display
		printf("\r");
		printf("%6f  |", rightrads);
		printf("%6f  |", leftrads);
		fflush(stdout);
		
		//If stay at reference mode enabled
		if(modeflag==0){

			if(rightpos>0){
				if(rc_set_motor(rightchannel, rightpolarity*k*rightrads))
				{
					fprintf(stderr,"RIGHT MOTOR ERROR: Shutting down!\n");
					return -1;
				}
				
			}
			if(rightpos<0){
				if(rc_set_motor(rightchannel, rightpolarity*k*rightrads))
				{
					fprintf(stderr,"RIGHT MOTOR ERROR: Shutting down!\n");
					return -1;
				}
				
			}	
			
			rc_set_led(GREEN, ON);
			rc_set_led(RED, OFF);
		}
		else if(modeflag==1){
		//else if(rc_get_state()==PAUSED){
			// Follow the undriven wheel mode
			
			int rightpos=rc_get_encoder_pos(rightchannel);
			int leftpos=rc_get_encoder_pos(leftchannel);	
			
			float rightrads=2*3.14159265358979323*rightpos/gearboxratio;
			float leftrads=2*3.14159265358979323*leftpos/gearboxratio;		
				
			float posdiff=rightrads+leftrads;	
				
				
			if((posdiff)>0)
			{
				if(rc_set_motor(rightchannel, rightpolarity*k*(posdiff)))
				{
					fprintf(stderr,"RIGHT MOTOR ERROR: Shutting down!\n");
					return -1;
				}
				
			}
			if((posdiff)<0)
			{
				if(rc_set_motor(rightchannel, rightpolarity*k*(posdiff)))
				{
					fprintf(stderr,"RIGHT MOTOR ERROR: Shutting down!\n");
					return -1;
				}
				
			}
			
			rc_set_led(GREEN, OFF);
			rc_set_led(RED, ON);
		}
		// always sleep at some point
		usleep(10000);
	}
	
	// exit cleanly
	rc_cleanup(); 
	return 0;
}


/*******************************************************************************
* void on_pause_released() 
*	
* Make the Pause button toggle between paused and running states.
*******************************************************************************/
void on_pause_released(){
	// toggle betewen paused and running modes
	if(rc_get_state()==RUNNING)		rc_set_state(PAUSED);
	else if(rc_get_state()==PAUSED)	rc_set_state(RUNNING);
	if (modeflag==0) modeflag=1;
	else if (modeflag==1) modeflag=0;
	return;
}

/*******************************************************************************
* void on_pause_pressed() 
*
* If the user holds the pause button for 2 seconds, set state to exiting which 
* triggers the rest of the program to exit cleanly.
*******************************************************************************/
void on_pause_pressed(){
	int i=0;
	const int samples = 100;	// check for release 100 times in this period
	const int us_wait = 2000000; // 2 seconds
	
	// now keep checking to see if the button is still held down
	for(i=0;i<samples;i++){
		rc_usleep(us_wait/samples);
		if(rc_get_pause_button() == RELEASED) return;
	}
	printf("long press detected, shutting down\n");
	rc_set_state(EXITING);
	return;
}
