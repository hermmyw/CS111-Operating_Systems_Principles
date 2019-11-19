#!/usr/bin/python
# Name: Hermmy Wang
# ID: 704978214
# Email: hermmyw@hotmail.com

import sys
import csv


# define constant
NONE = 0
FREE = 1
USED = 2
RESERVED = 3
DUPLICATE = 4
INVALID = 6

# error
def _error(string):
  sys.stderr.write(string+"\n")
  sys.exit(1)

# my data structures
total_blocks = 0
total_inodes = 0
bsize = 1024
isize = 0
first_data_block = 0
first_inode = 0
exit = 0

block_state = []
inode_state = []
parent = []
linkcount = []
refcount = []
dirent = []

free_inode_bm = []
alloc_inode_bm = []
free_blk_bm = []
alloc_blk_bm = []

# block consistency audits
def validBlk(blk):
  if blk < 0 or blk > total_blocks:
    return False
  return True

def isReserved(blk):
  if validBlk(blk) == True and blk < first_data_block:
    return True
  return False

def block_audit():
  #print free_blk_bm
  #print alloc_blk_bm
  for b in range(first_data_block, total_blocks):
    if free_blk_bm[b] == 0 and alloc_blk_bm[b] == 0:
      print "UNREFERENCED BLOCK",b
      exit = 2
    if free_blk_bm[b] == 1 and alloc_blk_bm[b] == 1:
      print "ALLOCATED BLOCK", b, "ON FREELIST"
      exit = 2

# i-node allocation audits
def validInode(inode):
  if inode >=2 and inode < total_inodes:
    return True
  return False

def inode_audit():
  for i in range(0, total_inodes):
    if free_inode_bm[i] == 1 and alloc_inode_bm[i] == 1:
      print "ALLOCATED INODE", i, "ON FREELIST"
      exit = 2
    elif free_inode_bm[i] == 0 and alloc_inode_bm[i] == 0:
      print "UNALLOCATED INODE", i, "NOT ON FREELIST"
      exit = 2
    elif alloc_inode_bm[i] == 1 and refcount[i] != linkcount[i]:
      print "INODE", i, "HAS", refcount[i], "LINKS BUT LINKCOUNT IS", linkcount[i]
      exit = 2

# directory consistency audits
def checkParent(d):
  if d == 2:   #root
    return 2
  for i in range(0, len(dirent)):
    if dirent[i]["ref"] == d and dirent[i]["name"] != "\'..\'" and dirent[i]["parent"] != d:
      return dirent[i]["parent"]
  return -1

def dir_audit():
  for i in range(0, len(dirent)):
    d = dirent[i]
    if validInode(d["ref"]) == False:
      print "DIRECTORY INODE", d["parent"], "NAME " + d["name"] + " INVALID INODE", d["ref"]
      exit = 2
    elif alloc_inode_bm[d["ref"]] == 0:
      print "DIRECTORY INODE", d["parent"], "NAME " + d["name"] + " UNALLOCATED INODE", d["ref"]
      exit = 2
    elif d["name"] == "\'.\'" and d["parent"] != d["ref"]:
      print "DIRECTORY INODE", d["parent"], "NAME \'.\' LINK TO INODE", d["ref"], "SHOULD BE", d["parent"]
      exit = 2
    elif d["name"] == "\'..\'" and d["ref"] != checkParent(d["parent"]):
      print "DIRECTORY INODE", d["parent"], "NAME \'..\' LINK TO INODE", d["ref"], "SHOULD BE", checkParent(d["parent"])
      exit = 2



def level_str(level):
  if level == 3 or level == 26:
    return "TRIPLE INDIRECT "
  elif level == 2 or level == 25:
    return "DOUBLE INDIRECT "
  elif level == 1 or level == 24:
    return "INDIRECT "
  else:
    return ""

def offset(level):
  if level == 3 or level == 26:
    return 12+bsize/4+bsize/4*bsize/4
  elif level == 2 or level == 25:
    return 12+bsize/4
  elif level == 1 or level == 24:
    return 12
  else:
    return 0

# main function
if __name__== "__main__":

    # check syntax
  if len(sys.argv) != 2:
    _error("Wrong number of argument")
  try:
    fd = open(sys.argv[1], "r")
  except IOError:
    _error("Can't open the file")

  # collect data from csv
  with open(sys.argv[1],"rb") as csvfile:
    linereader = csv.reader(csvfile)
    for line in linereader:
      if line[0] == "SUPERBLOCK":
        total_blocks = int(line[1])
        total_inodes = int(line[2])
        bsize = int(line[3])
        isize = int(line[4])
        first_inode = int(line[7])
        for i in range(0, total_blocks+1):
          free_blk_bm.append(0)
          alloc_blk_bm.append(0)
          block_state.append(NONE)
          parent.append({ "inode":0, "level":0 })
        for i in range(0, total_inodes+1):
          free_inode_bm.append(0)
          alloc_inode_bm.append(0)
          linkcount.append(0)
          refcount.append(0)
        for i in range(0, first_inode):
          alloc_inode_bm[i] = 1

      elif line[0] == "GROUP":
        first_data_block = int(line[8]) + isize*total_inodes/bsize
        for i in range(0, first_data_block):
          alloc_blk_bm[i] = 1

      elif line[0] == "BFREE":
        free_blk_bm[int(line[1])] = 1
        block_state[int(line[1])] = FREE
      elif line[0] == "IFREE":
        free_inode_bm[int(line[1])] = 1
      
      elif line[0] == "INODE":
        # skip symlinks with file size < 60B
        if line[2] == "s" and int(line[10]) < 60:
          continue

        # allocated inode has nonzero link count and nonzero mode
        if int(line[6]) != 0 and int(line[4]) != 0:
          alloc_inode_bm[int(line[1])] = 1
          linkcount[int(line[1])] = int(line[6])

        # check block pointers in each inode
        for i in range(12,27):
          if int(line[i]) == 0:
            continue
          #INVALID
          if validBlk(int(line[i])) == False:
            print "INVALID " + level_str(i) + "BLOCK " + line[i] + " IN INODE " + line[1] + " AT OFFSET", offset(i)
            exit = 2
          #RESERVED
          elif isReserved(int(line[i])) == True:
            print "RESERVED " + level_str(i) + "BLOCK " + line[i] + " IN INODE " + line[1] + " AT OFFSET", offset(i)
            exit = 2
          else:
            #DUPLICATE
            if block_state[int(line[i])] == USED or block_state[int(line[i])] == DUPLICATE:
              block_state[int(line[i])] = DUPLICATE
              print "DUPLICATE " + level_str(parent[int(line[i])]["level"]) + "BLOCK " + line[i] + " IN INODE " + parent[int(line[i])]["inode"] + " AT OFFSET", offset(parent[int(line[i])]["level"])
              print "DUPLICATE " + level_str(i) + "BLOCK " + line[i] + " IN INODE " + line[1] + " AT OFFSET", offset(i)
              exit = 2
            else:
              block_state[int(line[i])] = USED
              alloc_blk_bm[int(line[i])] = 1

            parent[int(line[i])] = { "inode":line[1], "level":i }  #inode=last_reference #level=24~26


      elif line[0] == "DIRENT":
        if validInode(int(line[3])) == True:
          refcount[int(line[3])] += 1
        dirent.append({ "parent":int(line[1]), "ref":int(line[3]), "offset":int(line[2]), "name":line[6] })


      elif line[0] == "INDIRECT":
        if validBlk(int(line[5])) == False:
          print "INVALID " + level_str(int(line[2])) + "INDIRECT BLOCK " + line[5] + " IN INODE " + line[1] + " AT OFFSET " + line[3]
          exit = 2
        elif isReserved(int(line[5])) == True:
          print "RESERVED " + level_str(int(line[2])) + "INDIRECT BLOCK " + line[5] + " IN INODE " + line[1] + " AT OFFSET " + line[3]
          exit = 2
        else:
          if block_state[int(line[5])] == USED or block_state[int(line[5])] == DUPLICATE:
            block_state[int(line[5])] = DUPLICATE
            print "DUPLICATE " + level_str(parent[int(line[5])]["level"]) + "BLOCK " + line[5] + " IN INODE " + parent[int(line[5])]["inode"] + " AT OFFSET", offset(parent[int(line[5])]["level"])
            print "DUPLICATE " + level_str(i) + "BLOCK " + line[5] + " IN INODE " + line[1] + " AT OFFSET", line[3]
            exit = 2
          else:
            block_state[int(line[5])] = USED
            alloc_blk_bm[int(line[5])] = 1
          
          parent[int(line[5])] = { "inode" : line[1], "level" : int(line[2]) }
        


  # report audits
  block_audit()
  inode_audit()
  dir_audit() 
  sys.exit(exit)