#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#define ARGC_ERROR 1
#define FILE_ERROR 2
#define BUFLEN 256
#define FRAME_SIZE  256


//-------------------------------------------------------------------
unsigned int getpage(size_t x) { return (0xff00 & x) >> 8; }
unsigned int getoffset(unsigned int x) { return (0xff & x); }
unsigned int get_add(unsigned int x, unsigned int offset) { return (x << 8) | offset; }

void getpage_offset(unsigned int x) {
  unsigned int page = getpage(x);
  unsigned int offset = getoffset(x);
  printf("x is: %u, page: %u, offset: %u, address: %u, paddress: %u\n", x, page, offset,
         (page << 8) | getoffset(x), page * 256 + offset);
}

int main(int argc, const char * argv[]) {
  // page fault means not in page page_table
  // if you get a page that is not in apge table (i.e. -1) we read nextavailable from the backing BINARY_STORE
  // and use this data to find the page

  int next_available = 0;
  int page_faults = 0;
  int attempts = 0;
  unsigned int page_table[256];
  int visited[256];
  unsigned int physical_mem[256 * 256];
  unsigned int tlb[16];

  srand(time(0));

  for(int i = 0; i < 256; i++){
      page_table[i] = -1;
      visited[i] = 0;
  }

  FILE* fadd = fopen("addresses.txt", "r");
  if (fadd == NULL) { fprintf(stderr, "Could not open file: 'addresses.txt'\n");  exit(FILE_ERROR);  }

  FILE* fcorr = fopen("correct.txt", "r");
  if (fcorr == NULL) { fprintf(stderr, "Could not open file: 'correct.txt'\n");  exit(FILE_ERROR);  }

  FILE* fbin = fopen("BACKING_STORE.bin", "r");
  if(fbin == NULL) { fprintf(stderr, "Could not open file: 'BACKING_STORE.bin'\n"); exit(FILE_ERROR); }

  char buf[BUFLEN];
  unsigned int page, offset, physical_add, frame = 0;
  unsigned int logic_add;                  // read from file address.txt
  unsigned int virt_add, phys_add, value;  // read from file correct.txt

      // not quite correct -- should search page table before creating a new entry
      //   e.g., address # 25 from addresses.txt will fail the assertion
      // TODO:  add TLB code
  while (!feof(fadd)) {
    fscanf(fcorr, "%s %s %d %s %s %d %s %d", buf, buf, &virt_add,
           buf, buf, &phys_add, buf, &value);  // read from file correct.txt

    fscanf(fadd, "%d", &logic_add);  // read from file address.txt
    page = getpage(logic_add);
    offset = getoffset(logic_add);



    attempts++;
    if(page_table[page] == -1){
      page_faults++;
      page_table[page] = next_available++;
      char* p = &physical_mem[256 * page_table[page]]; // read into here with .bin file

      // todo update TLB
        //page_table[page] = physical_add;
        //visited[page] = 1;
    }

    fseek(fbin, sizeof(page_table[page]), SEEK_SET);
    physical_add = page_table[page] * FRAME_SIZE + offset;

    printf("Address from file is %d\n Physical Address %d \n", virt_add, physical_add);
    assert(physical_add == phys_add);
    // todo: read BINARY_STORE and confirm value matches read value from correct.txt
    printf("logical: %5u (page:%3u, offset:%3u) ---> physical: %5u -- passed\n", logic_add, page, offset, physical_add);
    if (frame % 5 == 0) { printf("\n"); }
  }
  fclose(fcorr);
  fclose(fadd);

  printf("ALL logical ---> physical assertions PASSED!\n");
  printf("We made %d page fault lookups, with %d page faults!\n\n", attempts, page_faults);
  printf("\n\t\t...done.\n");
  return 0;
}
