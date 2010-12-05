/*------------------- DUMMY BOOT LOADER --------------------------------------------*/
#define FLASH_BLOCK_SIZE   32
#define LOADER_RESERVED    getenv("PROGRAM_MEMORY")-24*FLASH_BLOCK_SIZE
#define BUFFER_LEN_LOD     46
#if FLASH_BLOCK_SIZE != getenv("FLASH_ERASE_SIZE")/2
  #error Wrong length of the Flash Block Size. getenv("FLASH_ERASE_SIZE")/getenv("FLASH_WRITE_SIZE")
#endif


#BUILD(INTERRUPT=FLASH_BLOCK_SIZE)   // Redirect Interrupt routine above first flash block
#ORG 4,5
void JumpToTheInterrupt()     // Jump to the Interrupt Handler
{ #asm GOTO FLASH_BLOCK_SIZE #endasm }
#ORG 6,FLASH_BLOCK_SIZE-1 {} // First Flash block is reserved


#ORG LOADER_RESERVED,getenv("PROGRAM_MEMORY")-1 auto=0
#SEPARATE
void dummy_main() // Main on the fix position
{
   reset_cpu();
}
