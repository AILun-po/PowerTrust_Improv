#include "DHT.h"
#include <cstdio>
#include <set>


int count=0;   


void generate_IDs()
{
	FILE* fp=fopen("IDs.txt","w");
	int size=std::pow(2,10);
	std::set<int> id_set;
	for(int i=0;i<100;i++)
	{
		srand(std::time(NULL));
		while(true)
		{
			int res=rand()%size;
			if(id_set.find(res)==id_set.end())
			{	
				fprintf(fp,"%d,",res);
				id_set.insert(res);
				break;
			}
			else
				continue;
		}
		if(i%20==0)
			fprintf(fp,"\n");
	}
	fclose(fp);
}

int id_list[100]
=
{49,198,300,445,560,18,936,99,558,392,827,273,44,177,660,972,1008,719,62,510,517,
973,968,784,515,887,547,826,529,743,331,154,935,575,809,644,409,670,953,794,849,
76,616,796,347,312,704,1,808,933,70,523,288,419,518,437,771,971,206,866,955,
932,386,460,814,119,958,561,241,858,926,45,156,618,581,360,263,450,185,141,595,
750,587,984,695,52,864,293,879,372,424,883,841,724,769,574,169,692,921,777};

int    main()   
{   
    DHT::chord a;
	
	Sleep(500);

	for(int i=0;i<10;i++)
	{
		//int id=a.generate_id();
		a.add_node(id_list[i]);
		std::cout<<i<<std::endl;
		Sleep(500);
	}

	//int x=a.get_reprsnttve_node()
	//	->find_successor(321);

	

	while(true)
		std::cout<<"aaa"<<std::endl;
	
	
	return 0;
}    



