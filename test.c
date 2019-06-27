#include<stdio.h>
#include<stdlib.h>
int cmp(int const a, int *b);            

int *func(int *n)
  {     
      int k = *n + 2;   //            
      int m = 100 + k;    
      return &m;
  }

int cmp(int const a, int *b)
{
	int n = a;
	if(n == 0){
		printf("0");
	}else if(n == 1){	
	
	    printf("1");                                                                                                                                          
           } 
           else if(n == 2){
           printf("2");
           }
           else{
           	printf("other");
           } 
    return 0;
}

int add(int *a, int *b)
{
	if(*a == *b){
		return 0;
	}
	else{ }
	return 1;
}

int main(int argc, const char **argv)
{
	int first = 10;
	int sec = 20;
	int *second = &sec;
	int n = 8;
	int *p = func(&n);
	int *s;
	s = func(&sec);
	cmp(n, *second);
	if(cmp(first, second) > 0){
		cmp(sec, *second);
		printf("yes");
	}else if(cmp(first, second) < 0){	
	
	    printf("no");                                                                                                                                          
           } 
           else{
           printf("no2");
           } 
    if(n > 8){
    	return 0;
    }
    else{
    	return 1;
    }
}