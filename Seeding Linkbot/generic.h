#define boolean int//this is the most important part of the code...
#define true 1
#define false 0
#define println(thing); printf(thing);printf("\n");


//UTILITY
float bound(float num, float max)
{
	if (num > max)
		return max;
	else if (num < -max)
		return -max;
	else
		return num;
}
int in_range(int input, int wanted, int fudge)
{
	if (wanted+fudge >= input && wanted-fudge <= input)
		return 1;
	else
		return 0;
}
float ET_distance(int port){
	return ((sqrt(100.0/analog10(port)))-2.2);
}

#define WAIT(thing); while(!(thing)){msleep(1);}
#define LIMIT(thing,time); {double _tmptime = seconds()+time; while(!(thing) && (_tmptime > seconds())){msleep(1);}}
#define gmpc(port) get_motor_position_counter(port)

//LIGHT START
#define light_s() analog10(sensor)
void light_start(int sensor)
{
	WAIT(!(b_button() || c_button()));
	extra_buttons_hide();
	set_a_button_text("-");
	set_a_button_text("arm");
	set_a_button_text("skip");
	int max = 0,min = 9999,curr = 0,avg = 0;
	display_clear();
	display_printf(0,0,"Max:");
	display_printf(0,1,"Min:");
	display_printf(0,2,"Curr:");
	display_printf(0,3,"avg:");
	while(!b_button() && !c_button())//INTERACTION BUTTON
	{
		curr = light_s();
		if (curr > max)
		{max = curr;}
		if (curr < min)
		{min = curr;}
		avg = (max+min)/2;
		display_printf(5,0,"%d   ",max);
		display_printf(5,1,"%d   ",min);
		display_printf(6,2,"%d   ",curr);
		display_printf(5,3,"%d   ",avg);
		if (curr > avg)
		{
			display_printf(10,5,"XX");
		}
		else
		{
			display_printf(10,5,"OO");
		}
		msleep(50);
	}
	display_clear();
	display_printf(0,4,"Prepared to begin: c to skip");
	while(light_s() > avg && !(c_button()))
	{
		msleep(50);
	}
}

void brick()//the title says it all...
{
	while(1)
	{
		printf("%d", rand());
		//uncomment the next line only if you're a wimp...or don't actually want to brick the link XD
		//msleep(1);
	}
}

//TIMING
long _start_time=0;//just in case someone forgets start-->will still work ish
void start()
{
	_start_time = systime();
}
float curr_time()
{
	return (systime()-_start_time)/1000.0;
}
void now()     
{
	printf("now %f\n",curr_time());
}
void nowstr(char *s)
{
	printf("%s %f\n",s,curr_time());
}


void wait_till(float t)
{
	now();
	msleep((long)((t-curr_time())*1000));
}

//SERVOS
//servo_off(int) is replaced with the KIPR library function disable_servo(int)
void servo_set(int port,int end,float time)//,float increment)
{//position is from 0-2047
	float increment = .01;
	//printf("servo %d",port);
	float curr,start = get_servo_position(port);
	float i = ((end-start)/time)*increment;
	curr = start;
	if (start > end)
	{
		while(curr > end)
		{
			//printf("%f\n",curr);
			set_servo_position(port,curr);
			curr+=i;
			msleep((long)(increment*1000));
		}
	}
	else if (start < end)
	{
		while(curr < end)
		{
			//printf("%f\n",curr);
			set_servo_position(port,curr);
			curr+=i;
			msleep((long)(increment*1000));
		}
	}
	set_servo_position(port,end);
}

//camera stuff

int cam_area(int channel){//returns largest blob in channel, or 0 if none
	if (get_object_count(channel) > 0){
		return get_object_area(channel,0);
	}
	return 0;
}
void update_wait(){//updates the camera, and waits until success
	while(!camera_update()) msleep(1);
}

int currstate;
#define state(State) if (currstate == State)
#ifndef MENUa
   #define next(State); {currstate = State;}
#else//If menu code is in use, print the time of each state switch
	extern struct menuitem menu[];
	void next(int State) {
		int i;
		currstate = State;
		i = -1;
		while (!strcmp(menu[++i].name,"FIN")){
			if (menu[i].snum==State){
				nowstr(menu[i].name);
				return;
			}
		}
		now();
}
#endif


int getabbutton(){//returns 0,1 on a,b
	WAIT(!(a_button() || b_button()));
	WAIT(a_button() || b_button());
	if (a_button()) return 0;
	if (b_button()) return 1;
	printf("ERROR!");beep();
	msleep(2000);beep();
	return 0;//if something broke
}
int getabcbutton(){//returns 0,1,2 on a,b,c
	WAIT(!(a_button() || b_button() || c_button()));
	WAIT(a_button() || b_button() || c_button());
	if (a_button()) return 0;
	if (b_button()) return 1;
	if (c_button()) return 2;
	printf("ERROR!");beep();
	msleep(2000);beep();
	return 0;//if something broke
}

void reset_buttons()
{
	extra_buttons_hide();
	set_a_button_text("A");
	set_b_button_text("B");
	set_c_button_text("C");
}

int wait_time;
void wait_shutdown()
{
	msleep(wait_time);
	println("ended based off time");
	_exit(0);//this throws a warning, but hey, it still compiles, so whatevs
}
void shutdownin(float time)//cause raisins.
{
	wait_time=(int)(time*1000);
	thread tw=thread_create(wait_shutdown);
	thread_start(tw);
}

float sign(float input)//returns 1 for positive, 0 for 0, -1 for negative
{
	if(input>0)
		return 1.;
	if(input<0)
		return -1.;
	return 0.;//implied else
}

float my_abs(float input)//because apparently kipr broke abs...
{
	if(input<0)
		return -input;
	return input;
}

float input_float(int x, int y)//x and y are where the display_printf starts
{
	int digits=0;//how many digits have been inputted
	int curr_digit=0;//what number is selected right now
	float curr_number=0;//current output
	int digits_after_dec=-1;//number of digits after the decimal point-->-1 means no decimal point yet
	extra_buttons_show();
	set_a_button_text("-");//move curr_digit down by one
	set_c_button_text("+");//up
	set_x_button_text("backspace");//delete the last digit
	set_y_button_text(".");//decimal point
	set_z_button_text("enter");//done with input
	while(1)
	{
		set_b_button_text((char*)(curr_digit));//put in current selection
		WAIT(a_button()||b_button()||c_button()||x_button()||y_button()||z_button());
		if(a_button())
			curr_digit--;//I'll sanitize the result at the end of the loop
		else if(b_button())
		{
			if(digits_after_dec<0)//no dec point yet
			{
				curr_number*=10;
				curr_number+=(float)(curr_digit);
			}
			else//has a dec point
			{
				curr_number+=((float)(curr_digit))/exp10(digits_after_dec+1);//trust me, it works
				digits_after_dec++;//has an extra digit after the decimal point
			}
			display_printf(x+digits, y, "%d", curr_digit);
			digits++;
		}
		else if(c_button())
			curr_digit++;
		else if(x_button())
		{
			if(digits>0)//has a digit to delete-->otherwise, will ignore the button tap
			{
				if(digits_after_dec>0)//deleting a digit after the dec point
				{
					digits_after_dec--;
					curr_number=((float)((int)(curr_number*exp10(digits_after_dec))))/exp10(digits_after_dec);//don't question it...
				}
				else if(digits_after_dec==0)//deleting the decimal point
				{
					digits_after_dec=-1;//no more dec point
				}
				else//deleting a digit with no dec point
				{
					curr_number=(float)((int)(curr_number/10));//again, it works, just go with it
				}
				digits--;
				display_printf(x+digits, y, " ");//clear the input it deleted
			}
		}
		else if(y_button())
		{
			if(digits_after_dec<0)//doesn't have a decimal point yet-->can add one
			{//if not, will ignore button tap
				if(digits==0)//no digits yet-->add a leading 0
				{
					display_printf(x,y,"0");
					digits=1;
				}
				display_printf(x+digits, y, ".");
				digits_after_dec=0;//has a dec, but no numbers after it yet
				digits++;//counts as a digit for the purposes of printing in the right locations
			}
		}
		else//z
		{
			if(digits>0)//has put in a number-->if not, can't exit, so will ignore button tap
				break;//exit the loop-->get to the return
		}
		curr_digit=curr_digit%10;//this will get in in the range of 0-9
		WAIT(!(a_button()||b_button()||c_button()||x_button()||y_button()||z_button()));
		msleep(50);
	}
	return curr_number;
}

int input_int(int x, int y)//pretty much the same as input_float...
{
	int digits=0;//how many digits have been inputted
	int curr_digit=0;//what number is selected right now
	int curr_number=0;//current output
	extra_buttons_show();
	set_a_button_text("-");//move curr_digit down by one
	set_c_button_text("+");//up
	set_x_button_text("backspace");//delete the last digit
	set_y_button_text("");//nothing-->can't have a decimal point
	set_z_button_text("enter");//done with input
	while(1)
	{
		set_b_button_text((char*)(curr_digit));//put in current selection
		WAIT(a_button()||b_button()||c_button()||x_button()||z_button());
		if(a_button())
			curr_digit--;//I'll sanitize the result at the end of the loop
		else if(b_button())
		{
			curr_number*=10;
			curr_number+=(float)(curr_digit);
			display_printf(x+digits, y, "%d", curr_digit);
			digits++;
		}
		else if(c_button())
			curr_digit++;
		else if(x_button())
		{
			if(digits>0)//has a digit to delete-->otherwise, will ignore the button tap
			{
				curr_number=curr_number/10;//casts to an int automatically, cause both are ints
				digits--;
				display_printf(x+digits, y, " ");//clear the input it deleted
			}
		}
		else//z
		{
			if(digits>0)//has put in a number-->if not, can't exit, so will ignore button tap
				break;//exit the loop-->get to the return
		}
		curr_digit=curr_digit%10;//this will get in in the range of 0-9
		WAIT(!(a_button()||b_button()||c_button()||x_button()||z_button()));
		msleep(50);
	}
	return curr_number;
}
