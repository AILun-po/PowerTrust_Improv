#include "DHT.h"

namespace DHT
{
	int flags::gen_id_failsure=-1;
	int flags::nil_node_id=-1;
	int flags::finger_table_begin_idx=0;
	int flags::fix_fingers_start=-1;
	





	VOID CALLBACK timer_proc
		(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime)
	{
		chord* p_this=(chord* )idEvent;
		
		if(p_this->addr_map.empty())
			return;
		
		
		static std::map<int,chord::node*>::iterator it
			=p_this->addr_map.begin();
			
		if(it==p_this->addr_map.end())
			it=p_this->addr_map.begin();
			
		it->second->stablilize();
		for(int i=0;i<p_this->params.m;i++)
			it->second->fix_fingers();
		it->second->check_pre();
		it++;
	}
	
	DWORD CALLBACK timer_thread(PVOID pvoid)   
	{   
		MSG  msg;   
		PeekMessage(&msg,NULL,WM_USER,WM_USER,PM_NOREMOVE);   

		chord* p_this=(chord* )pvoid;
		

		UINT  timerID=SetTimer(NULL,NULL,
			p_this->params.update_freq,timer_proc);   
		//UINT  timerID=SetTimer(NULL,NULL,1,timer_proc);   
		BOOL  bRet;   
    
		while((bRet = GetMessage(&msg,NULL,0,0))!=0)   
		{     
			if(bRet==-1)   
			{   
				//   handle   the   error   and   possibly   exit   
			}   
			else   
			{    
				TranslateMessage(&msg);
				msg.wParam=(WPARAM)pvoid;
				DispatchMessage(&msg);     
			}   
		}   

		KillTimer(NULL,timerID);   
		printf("thread   end   here\n");   
		return 0;   
	}


	int chord::generate_id()
	{
		if(addr_map.size()<size)
		{
		//If the chord is not full.
			while(true)
			{
				srand((unsigned)std::time(NULL));	
				int res=rand()%size;
				if(addr_map.find(res)==addr_map.end())
					return res;
			}
		}
		else
		//Return failsure flag.
			return flags::gen_id_failsure;
	}

	void chord::add_node(int id)
	{
		node* addr=
			new node(id,params.m,
			params.base,addr_map,cs);
		addr_map[addr->get_id()]=addr;
		addr->join(reprsnttv_node);
		reprsnttv_node=addr->get_id();
	}

	chord::chord()
	{
		//Size is 2^m,and the node id
		//belongs to [0,2^m-1].
		size=std::pow(params.base,params.m);
		
		node* addr=
			new node(generate_id(),params.m,
			params.base,addr_map,cs);
		addr_map[addr->get_id()]=addr;
		reprsnttv_node=addr->get_id();

		InitializeCriticalSection(&cs);

		HANDLE   hThread  =
		CreateThread(NULL,0,timer_thread,this,0,NULL);
	}

	chord::node::node(int id,int m,int base,
				std::map<int,node*>& addr_map,
				CRITICAL_SECTION& cs)
				:addr_map(addr_map),cs(cs)
	{
		id_pre=flags::nil_node_id;
		id_n=id;
		id_suc=id_n;
		this->m=m;
		this->base=base;
		finger=std::vector<int>(m,id_n);
	}
	chord::node::~node()
	{
		finger.~vector();
	}
	chord::node::node(const node& n)
		:addr_map(n.addr_map),cs(n.cs)
	{
		id_pre=n.id_pre;
		id_n=n.id_n;
		id_suc=n.id_suc;
		finger=n.finger;
		m=n.m;
		base=n.base;
	}
	int chord::node::get_id()
	{
		return id_n;
	}

	int chord::node::find_successor_raw(int id)
	{
		if(id>this->id_n && id<=id_suc)
			return id_suc;
		else
		{
			int n0=closest_pre_node(id);
			if(n0!=id_n)
				return addr_map[n0]->find_successor_raw(id);
			else
				return id_n>id?(id_n):(flags::nil_node_id);
				//return id_n;
		}
	}

	int chord::node::find_successor(int id)
	{
		EnterCriticalSection(&cs);

		if(id>this->id_n && id<=id_suc)
		{
			LeaveCriticalSection(&cs);
			return id_suc;
		}
		else
		{
			int n0=closest_pre_node(id);
			
			if(n0!=id_n)
			{
				LeaveCriticalSection(&cs);
				return addr_map[n0]->find_successor(id);
			}
			else
			{
				LeaveCriticalSection(&cs);
				return id_n>id?(id_n):(flags::nil_node_id);
				//return id_n;
			}
		}

	}
	int chord::node::closest_pre_node(int id)
	{
		EnterCriticalSection(&cs);

		for(int i=this->m-1;i>=0;i--)
			if(finger[i]>id_n && finger[i]<id)
			{
				LeaveCriticalSection(&cs);
				return finger[i];
			}

		LeaveCriticalSection(&cs);
		return id_n;

	}

	void chord::node::join(int id)
	{
		EnterCriticalSection(&cs);

		id_pre=flags::nil_node_id;
		id_suc=addr_map[id]->
			find_successor_raw(id_n);

		LeaveCriticalSection(&cs);
	}

	void chord::node::stablilize()
	{
		EnterCriticalSection(&cs);
		
		int x=addr_map[id_suc]->id_pre;
		
		//Check if x belongs to (id_n,id_suc).
		if(x>id_n && x<id_suc)
			id_suc=x;

		addr_map[id_suc]->notify_raw(id_n);

		LeaveCriticalSection(&cs);
	}

	void chord::node::notify_raw(int id)
	{
		if(id_pre==flags::nil_node_id
			|| (id>id_pre && id<id_n))
			id_pre=id;
	}

	void chord::node::notify(int id)
	{
		EnterCriticalSection(&cs);

		//Check if id belongs to (id_pre,id_n).
		if( (id_pre==flags::nil_node_id)
			|| (id>id_pre && id<id_n))
			id_pre=id;

		LeaveCriticalSection(&cs);
	}

	void chord::node::fix_fingers()
	{
		EnterCriticalSection(&cs);
		
		static int next=flags::fix_fingers_start;
		next=next+1;
		if(next>=m)
			next=flags::finger_table_begin_idx;

		//finger[next] = find_succesor(n+2^(next-1))
		finger[next]=
			find_successor_raw(id_n+std::pow(base,next-1));
		
		LeaveCriticalSection(&cs);
	}

	void chord::node::check_pre()
	{
		EnterCriticalSection(&cs);

		if(addr_map.find(id_pre)==addr_map.end())
			id_pre=flags::nil_node_id;

		LeaveCriticalSection(&cs);
	}







}