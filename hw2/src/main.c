#include "debug.h"
#include "utf.h"
#include "wrappers.h"
#include <stdlib.h>

int
main(int argc, char *argv[])
{
  int infile, outfile, in_flags, out_flags;
  parse_args(argc, argv);
  check_bom();
  print_state();
  in_flags = O_RDONLY;
  out_flags = O_WRONLY | O_CREAT;
  infile = Open(program_state->in_file, in_flags);
  outfile = Open(program_state->out_file, out_flags);
  struct stat infile_stat;
  struct stat outfile_stat;
  fstat(infile,&infile_stat);
  fstat(outfile,&outfile_stat);
  debug("%d",infile_stat.st_ino == outfile_stat.st_ino);
  if(infile_stat.st_ino == outfile_stat.st_ino){
  free(program_state);
  close(outfile);
  close(infile);
  return EXIT_FAILURE;
  }
  lseek(SEEK_SET, program_state->bom_length, infile); /* Discard BOM */
  get_encoding_function()(infile, outfile);
  if(program_state != NULL) {
    //close(program_state);
    free(program_state);
  }
  //I think this is how this works
  close(outfile);
  close(infile);
  return EXIT_SUCCESS;
}
