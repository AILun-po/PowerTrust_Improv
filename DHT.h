#ifndef M_DHT_FILE
#define M_DHT_FILE

//#include <windows.h>  
#include <afxwin.h>
#include <conio.h> 
#include <iostream>
#include <ctime>
#include <cmath>
#include <vector>
#include <list>
#include <map>


namespace DHT
{
	namespace flags
	{
		extern int gen_id_failsure;
		extern int nil_node_id;
		extern int finger_table_begin_idx;
		extern int fix_fingers_start;
	}
	




	

	class chord
	{
	protected:
		class node;
		
		CRITICAL_SECTION cs;

		struct params
		{
			int m;
			int base;
			int update_freq;
			params()
			{
				m=15;
				base=2;
				update_freq=10;
			}
		}params;

		
		int size;


		
		int reprsnttv_node;//ID of representative node.
	public:

		//Check if the x lies in the range
		//(begin,end) or [begin,end] or ...
		//allow_e1=true means [begin,
		//allow_e2=true means ,end].
		static bool is_lying_in
			(int x,int begin,int end,
			bool allow_e1,bool allow_e2,int size);

		friend VOID CALLBACK timer_proc
			(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime);

		
		friend DWORD CALLBACK timer_thread(PVOID pvoid);


		std::map<int,node*> addr_map;

		//Generate id in the range [0,2^m-1]
		int generate_id();

		class node
		{
		protected:
			std::vector<int> finger;
			int id_suc;
			int id_pre;
			int id_n;
			int m;
			int base;
			int size;

			std::map<int,node*>& addr_map;
			CRITICAL_SECTION& cs;

		public:
			node(int id,int m,int base,int size,
				std::map<int,node*>& addr_map,
				CRITICAL_SECTION& cs);
			~node();
			node(const node& n);
			int get_id();

			
			int closest_pre_node(int id);

			void join(int id);


			//1.Check if pre of suc
			//could be updated by id_n.
			//2.Check if suc could be
			//updated by pre of suc.
			void stablilize();

			//Without operations for
			//critical section
			void notify_raw(int id);
			

			//Check if pre of id_n
			//could be updated by id.
			void notify(int id);

			//Without operations for
			//critical section
			int find_successor_raw(int id);

			int find_successor(int id);

			void fix_fingers();

			void check_pre();
		};


		chord();
		~chord()
		{
			addr_map.clear();
			TerminateThread(timer_thread,0);
		}
		void add_node(int id);

		node* get_reprsnttve_node()
		{
			return addr_map[reprsnttv_node];
		}
	};

















}

#endif