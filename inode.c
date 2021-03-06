#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "inode.h"
#include "volumes.h"
#include "drive.h"


void
read_inode (unsigned int inumber, struct inode_s *inode)
{
  read_block_n (current_volume, inumber, (unsigned char *) inode,
		sizeof (struct inode_s));
}

void
write_inode (unsigned int inumber, const struct inode_s *inode)
{
  write_block_n (current_volume, inumber, (unsigned char *) inode,
		 sizeof (struct inode_s));
}



unsigned int
create_inode (enum file_type_e type)
{
  /* Initialisation de la structure */
  struct inode_s inode;
  inode.ind_type = type;
  inode.ind_size = 0;
  int i;
  for (i = 0; i < NDIRECT; ++i)
    inode.ind_direct[i] = 0;
  inode.ind_indirect = 0;
  inode.ind_2indirect = 0;
  /* Ecriture de l'inode */
  unsigned int inumber = new_block ();
  if (inumber == BLOC_NULL)	/* Disque plein */
    return 0;
  write_inode (inumber, &inode);
  return inumber;
}


int
delete_inode (unsigned int inumber)
{
  struct inode_s inode;
  read_inode (inumber, &inode);
  /* Direct blocks */
  free_blocks (inode.ind_direct, NDIRECT);
  /* Indirect blocks */
  if (inode.ind_indirect != BLOC_NULL)
    {
      unsigned int indirect[NBPB];
      read_block_n (current_volume, inode.ind_indirect,
		    (unsigned char *) &indirect,
		    (unsigned int) (NBPB * sizeof (unsigned int)));
      free_blocks (indirect, NBPB);
      free_block (inode.ind_indirect);
    }
  /* Double indirect blocks */
  if (inode.ind_2indirect != BLOC_NULL)
    {
      unsigned int dindirect[NBPB];
      read_block_n (current_volume, inode.ind_2indirect,
		    (unsigned char *) &dindirect,
		    (unsigned int) (NBPB * sizeof (unsigned int)));
      unsigned int i;
      for (i = 0; i < NBPB; ++i)
	{
	  if (dindirect[i] != BLOC_NULL)
	    {
	      unsigned int ddindirect[NBPB];
	      read_block_n (current_volume, dindirect[i],
			    (unsigned char *) &ddindirect,
			    (unsigned int) (NBPB * sizeof (unsigned int)));
	      free_blocks (ddindirect, NBPB);
	    }
	}
      free_blocks (dindirect, NBPB);
      free_block (inode.ind_2indirect);

    }
  free_block (inumber);
  return 0;
}





unsigned int
vbloc_of_fbloc (unsigned int inumber, unsigned int fbloc, bool_t do_allocate)
{
  struct inode_s inode;
  read_inode (inumber, &inode);
  if (fbloc < NDIRECT)
    {
      if (do_allocate && inode.ind_direct[fbloc] == 0)
	{
	  inode.ind_direct[fbloc] = new_block ();
	  write_inode (inumber, &inode);
	}
      return inode.ind_direct[fbloc];
    }
  else
    return 0;
}
