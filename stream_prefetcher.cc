#include "cache.h"
#include <vector>
#include <map>
namespace
{
struct monitor{

uint64_t stage;
uint64_t timestamp;
uint64_t starting;
uint64_t end;
int direction;
		    };

constexpr static int PREFETCH_DEGREE   = 3;
constexpr static int PREFETCH_DISTANCE = 10;
std::map<CACHE*, std::vector<struct monitor> > specific;
}

void CACHE::prefetcher_initialize() { }
//What is the job of this function?
uint32_t CACHE::prefetcher_cache_operate(uint64_t addr, uint64_t ip, uint8_t cache_hit, bool useful_prefetch, uint8_t type, uint32_t metadata_in)
{ 
  addr = addr >> LOG2_BLOCK_SIZE;
  //because everything has been converted into a cache line.
  if( cache_hit == 1 ) return metadata_in;
  		
  		//prefetch_line((( addr + 1 ) << LOG2_BLOCK_SIZE), true, metadata_in);
  	     
  //My qeustion is what you should do when the cache hit happens. 
  //prefetch_line(pf_addr, true, metadata_in);		   
  int size = specific[this].size();
  int i = 0; 
  for(i = 0; i < size; i++)
{
	struct monitor region = specific[this][i];
if( region.stage == 2 )
 {
   if( ( addr >= region.starting && addr <= region.end ) || ( addr <= region.starting && addr >= region.end ) )
	{
		//This implies the miss happens in that region.
		//so we have to prefetch certain lines.
		for(int j = 1; j <= PREFETCH_DEGREE; j++)
		{	
			uint64_t pf_addr = region.end + ( j * (region.direction)); //what is the meaning of line?
			prefetch_line(pf_addr << LOG2_BLOCK_SIZE, true, metadata_in);				
		}
		specific[this][i].starting = region.starting +  PREFETCH_DEGREE * region.direction;
		specific[this][i].end = region.end +  PREFETCH_DEGREE * region.direction;
		specific[this][i].timestamp = current_cycle;
		break;
		
	}
   else
	{
		continue;
	}

 }	
else if( region.stage == 1 )
 {
   if((region.starting < region.end && region.end < addr)||(region.starting > region.end && region.end > addr))
	{
		if( region.direction == -1 && (addr > (region.starting - PREFETCH_DISTANCE) ))
		{
			//welcome to stage 2...
			specific[this][i].stage = 2;
			specific[this][i].end = region.starting +  PREFETCH_DEGREE * region.direction;
			specific[this][i].timestamp = current_cycle;
			break;					
		}
		else if( region.direction == 1 && ((addr - region.starting) < (PREFETCH_DISTANCE) ))
		{
			//welcome to stage 2...
			specific[this][i].stage = 2;
			specific[this][i].end = region.starting + PREFETCH_DEGREE * region.direction;
			specific[this][i].timestamp = current_cycle;
			break;			
		}
		
	}
	else
	{
		continue;
	}
 }
else if( region.stage == 0 )
 {
   if( (addr - region.starting ) != 0 && ( ( addr < (region.starting + PREFETCH_DISTANCE) ) &&  (addr > (region.starting - PREFETCH_DISTANCE) ) ))
	{	
	     if( addr > region.starting )
		{ specific[this][i].direction = 1; } 
		else if( addr < region.starting )
		{ specific[this][i].direction = -1; }
		
		specific[this][i].stage = 1;
		specific[this][i].end = addr;
		specific[this][i].timestamp  = current_cycle;
		break;
		  
	}
    else
	{
		continue;
	} 
 }

}

if( i != size ) {   return metadata_in; }
if( i == size )
{ 
  
  if( size == 64 )
	{     
		//find the victim;
		int minindex = 0;
		for(int j = 0; j < 64; j++)
		{ 
			if ( specific[this][j].timestamp < specific[this][minindex].timestamp ){ minindex = j;}
			
		}
		specific[this][minindex].stage = 0;
		specific[this][minindex].starting = addr;
		specific[this][minindex].timestamp = current_cycle;
		return metadata_in;
		
	}
  else
	{
		struct monitor naya;
		naya.stage = 0;
		naya.starting = addr;
		naya.timestamp = current_cycle;
		specific[this].push_back(naya);
		
		return metadata_in;
		
	}

} 

//if now the we haven't got any sort of monitoring region that fulfills the need.

  
  return metadata_in;
}

uint32_t CACHE::prefetcher_cache_fill(uint64_t addr, uint32_t set, uint32_t way, uint8_t prefetch, uint64_t evicted_addr, uint32_t metadata_in)
{
  return metadata_in;
}

void CACHE::prefetcher_cycle_operate() {}

void CACHE::prefetcher_final_stats() {}
