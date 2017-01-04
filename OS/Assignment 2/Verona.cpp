#include <iostream>
#include <string>
#include <sstream>
#include <pthread.h>
#include <semaphore.h> 
#include <unistd.h>

using namespace std;

#define NUM_THREADS     10

static pthread_mutex_t mut;
static pthread_mutex_t mut1;
static pthread_mutex_t mut2;
static int numberOfCapulets;
static int numberOfMontagues;
static int numberOfCWaits;
static int numberOfMWaits;
static sem_t CapuletSem;
static sem_t MontagueSem;
static sem_t WaitingCCount;
static sem_t WaitingMCount;

struct plazaData {
   		string family;
		string name;
		int arrival_time;
		int occupying_time;
	};

string input_data[10];

plazaData readInputData(string input_line){

	struct plazaData entry;
	string delimiter = " ";
	size_t pos = 0; 	
	string data;
	int i=0;

	while((pos=input_line.find(delimiter)) != string::npos){
  		data=input_line.substr(0,pos);
  		input_data[i]=data;
  		input_line.erase(0,pos+delimiter.length());
  		i++;		
	}

	input_data[i] = input_line;
	entry.family=input_data[0];
	entry.name=input_data[1];
	stringstream out1(input_data[2]);
	stringstream out2(input_data[3]);
	out1 >> entry.arrival_time;
	out2 >> entry.occupying_time;
	return entry;
}

void printPlazaData(struct plazaData entry1){

  pthread_mutex_lock(&mut);
  cout << entry1.family<<" " <<entry1.name <<" enters the plaza " << endl;  
  pthread_mutex_unlock(&mut);
  
  sleep(entry1.occupying_time);

  pthread_mutex_lock(&mut);
  cout << entry1.family<<" " <<entry1.name <<" leaves the plaza " << endl;
  pthread_mutex_unlock(&mut);
}

void *gotoPlaza(void *arg) {
   	
   	struct plazaData entry1;
   	entry1= *(struct plazaData*) arg;
   	
   	sleep(entry1.arrival_time);
   	
   	cout << entry1.family<<" " <<entry1.name <<" "<<" arrives to plaza at time "<<entry1.arrival_time << endl;
    
    sem_getvalue(&CapuletSem,&numberOfCapulets);
    sem_getvalue(&MontagueSem,&numberOfMontagues);

    if(entry1.family=="Capulet"){
      
      start1:
    
      if(numberOfMontagues == 0 ){
        
        if(numberOfCapulets ==0)
          pthread_mutex_lock(&mut1);

        sem_post(&CapuletSem);
        sem_getvalue(&CapuletSem,&numberOfCapulets);
        printPlazaData(entry1);
        sem_wait(&CapuletSem);
        sem_getvalue(&CapuletSem,&numberOfCapulets);

        if(numberOfCapulets==0)
          pthread_mutex_unlock(&mut1);

            
      }

      else{
        goto start1;
      }
    }
      
    else if (entry1.family=="Montague"){
      
      start2:
    
      if(numberOfCapulets==0 ){ 

        if(numberOfMontagues == 0)
          pthread_mutex_lock(&mut2);

        
        sem_post(&MontagueSem);
        sem_getvalue(&MontagueSem,&numberOfMontagues);
        printPlazaData(entry1);
        sem_wait(&MontagueSem);
        sem_getvalue(&MontagueSem,&numberOfMontagues);
        
        if(numberOfMontagues ==0)
          pthread_mutex_unlock(&mut2);

      }

      else{
        goto start2;
      }
    
    }
    
    pthread_exit( (void*) 0 );		
}

int main() {
	
	string input_line;
	pthread_t threads[NUM_THREADS];
  int rc;
  int thread_count=0;
  struct plazaData entry1[NUM_THREADS];
  sem_init(&CapuletSem, 0, 0);
  sem_init(&MontagueSem, 0, 0);
  sem_init(&WaitingCCount, 0, 0);
  sem_init(&WaitingMCount, 0, 0);

	while (getline(cin, input_line)) { 		

    entry1[thread_count]= readInputData(input_line);
    rc = pthread_create(&threads[thread_count], NULL, gotoPlaza, &entry1[thread_count]);
    
    if (rc)
      return -1;
    thread_count++;
  }
  
  for(int j=0;j<thread_count;j++){
    pthread_join(threads[j], NULL);	
  }

  return 0;
}