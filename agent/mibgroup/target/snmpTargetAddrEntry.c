/* snmpTargetAddrEntry MIB
   
   This file was generated by mib2c and is intended for use as a mib module
   for the ucd-snmp snmpd agent. Edited by Michael Baer

   last changed 2/2/99.
*/

#include <config.h>
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <stdlib.h>
#include <ctype.h>

#include "mibincl.h"
#include "snmpTargetAddrEntry.h"
#include "read_config.h"

#define snmpTargetAddrOIDLen 11 /*This is base+column, 
				  i.e. everything but index*/

oid snmpTargetAddrOID[snmpTargetAddrOIDLen] = {1,3,6,1,6,3,15,1,2,1,0};

static struct targetAddrTable_struct *aAddrTable=0;


/* Utility routines */


/* TargetAddrTable_create creates and returns a pointer
   to a targetAddrTable_struct with default values set */
struct targetAddrTable_struct 
*snmpTargetAddrTable_create(void)
{
  struct targetAddrTable_struct *newEntry;

  newEntry = (struct targetAddrTable_struct *)
    malloc(sizeof(struct targetAddrTable_struct));

  newEntry->name        = 0;

  newEntry->tDomainLen  = 0;
  newEntry->tAddress    = 0;
 
  newEntry->timeout     = 1500;
  newEntry->retryCount  = 3;

  newEntry->tagList     = strdup("");
  newEntry->params      = 0;
  newEntry->spinLock    = 0;

  newEntry->storageType = SNMP_STORAGE_NONVOLATILE;
  newEntry->rowStatus   = SNMP_ROW_NONEXISTENT;
  newEntry->next        = 0;

  return newEntry;
}  /* snmpTargetAddrTable_create */


/* TargetAddrTable_dispose frees the space allocated to a
   targetAddrTable_struct */
void snmpTargetAddrTable_dispose(struct targetAddrTable_struct *reaped)
{
  free(reaped->name);
  free(reaped->tAddress);
  free(reaped->tagList);
  free(reaped->params);
  
  free(reaped);
}  /* snmpTargetAddrTable_dispose  */


/* snmpTargetAddrTable_addToList adds a targetAddrTable_struct 
   to a list passed in. The list is assumed to be in a sorted order,
   low to high and this procedure inserts a new struct in the proper 
   location. Sorting uses OID values based on name. A new equal value 
   overwrites a current one. */
void snmpTargetAddrTable_addToList(
     struct targetAddrTable_struct *newEntry,
     struct targetAddrTable_struct **listPtr)
{
  static struct targetAddrTable_struct *curr_struct, *prev_struct;
  int    i, newOIDLen = 0, currOIDLen = 0;
  oid    newOID[128], currOID[128];
  
  /* if the list is empty, add the new entry to the top */
  if ( (prev_struct = curr_struct = *listPtr) == 0 ) {
    *listPtr = newEntry;
    return;
  }
  else {
    /* get the 'OID' value of the new entry */
    newOIDLen = strlen(newEntry->name);
    for(i=0; i < newOIDLen ;i++) {
      newOID[i] = newEntry->name[i];
    }

    /* search through the list for an equal or greater OID value */
    while (curr_struct != 0) {
      currOIDLen = strlen(curr_struct->name);
      for(i=0; i < currOIDLen ;i++) {
	currOID[i] = curr_struct->name[i];
      }

      i=snmp_oid_compare(newOID, newOIDLen, currOID, currOIDLen);
      if (i==0) {  /* Exact match, overwrite with new struct */
	newEntry->next = curr_struct->next;
	/* if curr_struct is the top of the list */
	if (*listPtr == curr_struct)  *listPtr = newEntry;
	else prev_struct->next = newEntry;
	snmpTargetAddrTable_dispose(curr_struct);
	return;
      }
      else if (i < 0) { /* Found a greater OID, insert struct in front of it.*/
	newEntry->next = curr_struct;
	/* if curr_struct is the top of the list */
	if (*listPtr == curr_struct) *listPtr = newEntry;
	else prev_struct->next = newEntry;
	return;
      }
      prev_struct = curr_struct;
      curr_struct = curr_struct->next;
    }
  }
  /* if we're here, no larger OID was ever found, insert on end of list */
  prev_struct->next = newEntry;
}  /* snmpTargeAddrTable_addToList  */


/* snmpTargetAddrTable_remFromList removes a targetAddrTable_struct 
   from the list passed in */
void snmpTargetAddrTable_remFromList(
     struct targetAddrTable_struct *oldEntry,
     struct targetAddrTable_struct **listPtr)
{
  struct targetAddrTable_struct *tptr;

  if ( (tptr = *listPtr) == 0 ) return;
  else if (tptr == oldEntry) {
    *listPtr = (*listPtr)->next;
    snmpTargetAddrTable_dispose(tptr);
    return;
  }
  else  {
    while (tptr->next != 0) {
      if (tptr->next == oldEntry) {
	tptr->next = tptr->next->next;
	snmpTargetAddrTable_dispose(oldEntry);
	return;
      }
      tptr = tptr->next;
    }
  }	
}  /* snmpTargetAddrTable_remFromList  */


/* lookup OID in the link list of Addr Table Entries */
struct targetAddrTable_struct *
search_snmpTargetAddrTable(
     oid    *baseName,
     int    baseNameLen,
     oid    *name,
     int    *length,
     int    exact)
{
   static struct targetAddrTable_struct *temp_struct;
   int    i, myOIDLen = 0;
   oid    newNum[128];

   /* lookup entry in addrTable linked list, Get Current MIB ID */
   memcpy(newNum, baseName, baseNameLen*sizeof(oid));
  
   for( temp_struct = aAddrTable; temp_struct != 0; temp_struct = temp_struct->next) {
     for(i=0; i < strlen(temp_struct->name) ;i++) {
       newNum[baseNameLen+i] = temp_struct->name[i];
     }
     myOIDLen = baseNameLen+strlen(temp_struct->name);
     i=snmp_oid_compare(name, *length, newNum, myOIDLen);
     /* Assumes that the linked list sorted by OID, low to high */
     if ( (i==0 && exact!=0) || (i<0 && exact==0) ) {
       if (exact == 0) {
	 memcpy(name, newNum, myOIDLen*sizeof(oid));
	 *length = myOIDLen;
       }
       return temp_struct;
     }
   }
   return(0);
}  /* search_snmpTargetAddrTable  */


/* snmpTargetAddr_rowStatusCheck is boolean funciton that  checks 
   the status of a row's values in order to determine whether
   the row should be notReady or notInService  */
int snmpTargetAddr_rowStatusCheck(struct targetAddrTable_struct *entry)
{
  if ( (entry->tDomainLen == 0) || (entry->tAddress == 0) ||
       (entry->params == 0)  )
    return 0;
  else
    return 1;
}  /* snmtpTargetAddrTable_rowStatusCheck */


/* Init routines */


void init_snmpTargetAddrEntry(void) {
  aAddrTable = 0;
  snmpd_register_config_handler("targetAddr", snmpd_parse_config_targetAddr,
				0, "");
}  /* init_snmpTargetAddrEntry */


int snmpTargetAddr_addName(
     struct targetAddrTable_struct *entry,
     char   *cptr)
{
  int    len;
  if (cptr == 0) {
    DEBUGP("ERROR snmpTargetAddrEntry: no name in config string\n");
    return(0);
  }
  else {
    len = strlen(cptr);    
    /* spec check for string 1-32 */
    if (len < 1 || len > 32)  {
      DEBUGP("ERROR snmpTargetAddrEntry: name out of range in config string\n");
      return(0);
    }
    entry->name = (char *)malloc(sizeof(char) * (len + 1));
    strncpy(entry->name, cptr, len);
    entry->name[len] = '\0';
  }
  return(1);
} /* addName */
  

int snmpTargetAddr_addTDomain(
     struct targetAddrTable_struct *entry,
     char   *cptr)
{
  int len=128;
  
  if (cptr == 0) {
    DEBUGP("ERROR snmpTargetAddrEntry: no tDomain in config string\n");
    return(0);
  }

  if ( !read_objid(cptr, entry->tDomain, &len) ) {
    DEBUGP("ERROR snmpTargetAddrEntry: tDomain unreadable in config string\n");
    return(0);
  }

  /* spec check for oid 1-128 */
  if (len < 1 || len > 128)  {
    DEBUGP("ERROR snmpTargetAddrEntry: tDomain out of range in config string\n");
    return(0);
  }
  
  entry->tDomainLen = len;
  return(1);
} /* snmpTargetAddr_addTDomain */


int snmpTargetAddr_addTAddress(
     struct targetAddrTable_struct *entry,
     char   *cptr)
{
  int    len;
  if (cptr == 0) {
    DEBUGP("ERROR snmpTargetAddrEntry: no tAddress in config string\n");
    return(0);
  }
  else {
    len = strlen(cptr);    
    /* spec check for string 1-32 */
    /*    if (len < 1 || len > 32)  {
	  DEBUGP("ERROR snmpTargetAddrEntry: name out of range in config string\n");
	  return(0);
      } */
    free(entry->tAddress);
    entry->tAddress = (char *)malloc(sizeof(char) * (len + 1));
    strncpy(entry->tAddress, cptr, len);
    entry->tAddress[len] = '\0';
  }
  return(1);
} /* snmpTargetAddr_addTAddress */
  
  
int snmpTargetAddr_addTimeout(
     struct targetAddrTable_struct *entry,
     char   *cptr)
{
  if (cptr == 0) {
    DEBUGP("ERROR snmpTargetParamsEntry: no Timeout in config string\n");
    return(0);
  }
  else if (!(isdigit(*cptr))) {
    DEBUGP("ERROR snmpTargeParamsEntry: Timeout is not a digit in config string\n");
    return(0);
  }
  /* check Timeout >= 0 */
  else if ( (entry->timeout = (int)strtol(cptr, (char **)NULL, 0)) < 0) {
    DEBUGP("ERROR snmpTargeParamsEntry: Timeout out of range in config string\n");
    return(0);
  }
  return(1);
}  /* snmpTargetAddr_addTimeout  */
  
  
int snmpTargetAddr_addRetryCount(
     struct targetAddrTable_struct *entry,
     char   *cptr)
{
  if (cptr == 0) {
    DEBUGP("ERROR snmpTargetParamsEntry: no Retry Count in config string\n");
    return(0);
  }
  else if (!(isdigit(*cptr))) {
    DEBUGP("ERROR snmpTargeParamsEntry: Retry Count is not a digit in config string\n");
    return(0);
  }
  /* spec check 0..255 */
  else {
    entry->retryCount = (int)strtol(cptr, (char **)NULL, 0);
    if ( (entry->retryCount < 0) ||
	 (entry->retryCount > 255) )  {
      DEBUGP("ERROR snmpTargeParamsEntry: Retry Count is out of range in config string\n");
      return(0);
    }
  }
  return(1);
}  /* snmpTargetAddr_addRetryCount  */

 
int snmpTargetAddr_addTagList(
     struct targetAddrTable_struct *entry,
     char   *cptr)
{
  int    len;
  if (cptr == 0) {
    DEBUGP("ERROR snmpTargetAddrEntry: no tag list in config string\n");
    return(0);
  }
  else {
    len = strlen(cptr);    
    /* spec check for string 0-255 */
    if (len < 0 || len > 255)  {
      DEBUGP("ERROR snmpTargetAddrEntry: tag list out of range in config string\n");
      return(0);
    } 
    free(entry->tagList);
    entry->tagList = (char *)malloc(sizeof(char) * (len + 1));
    strncpy(entry->tagList, cptr, len);
    entry->tagList[len] = '\0';
  }
  return(1);
} /* snmpTargetAddr_addTagList */
  
 
int snmpTargetAddr_addParams(
     struct targetAddrTable_struct *entry,
     char   *cptr)
{
  int    len;
  if (cptr == 0) {
    DEBUGP("ERROR snmpTargetAddrEntry: no params in config string\n");
    return(0);
  }
  else {
    len = strlen(cptr);    
    /* spec check for string 1-32 */
    if (len < 1 || len > 32)  {
      DEBUGP("ERROR snmpTargetAddrEntry: params out of range in config string\n");
      return(0);
    } 
    entry->params = (char *)malloc(sizeof(char) * (len + 1));
    strncpy(entry->params, cptr, len);
    entry->params[len] = '\0';
  }
  return(1);
} /* snmpTargetAddr_addParams */

  
int snmpTargetAddr_addStorageType(
     struct targetAddrTable_struct *entry,
     char   *cptr)
{
  char   buff[1024];

  if (cptr == 0) {
    DEBUGP("ERROR snmpTargetAddrEntry: no storage type in config string\n");
    return(0);
  }
  else if (!(isdigit(*cptr))) {
    DEBUGP("ERROR snmpTargetAddrEntry: storage type is not a digit in config string\n");
    return(0);
  }
  /* check that storage type is a possible value */
  else if ( ((entry->storageType = (int)strtol(cptr, (char **)NULL, 0)) 
	     != SNMP_STORAGE_OTHER) &&
	    (entry->storageType != SNMP_STORAGE_VOLATILE) && 
	    (entry->storageType != SNMP_STORAGE_NONVOLATILE)  &&
	    (entry->storageType != SNMP_STORAGE_PERMANENT) && 
	    (entry->storageType != SNMP_STORAGE_READONLY) )  {
    sprintf(buff,"ERROR snmpTargetAddrEntry: storage type not a valid value of other(%d), volatile(%d), nonvolatile(%d), permanent(%d), or readonly(%d) in config string.\n", SNMP_STORAGE_OTHER, SNMP_STORAGE_VOLATILE, SNMP_STORAGE_NONVOLATILE, SNMP_STORAGE_PERMANENT, SNMP_STORAGE_READONLY);
    DEBUGP(buff);

    return(0);
  }
  return(1);
}  /* snmpTargetAddr_addStorageType */
  
  
int snmpTargetAddr_addRowStatus(
     struct targetAddrTable_struct *entry,
     char   *cptr)
{
  char buff[1024];

  if (cptr == 0) {
    DEBUGP("ERROR snmpTargetAddrEntry: no Row Status in config string\n");
    return(0);
  }
  else if (!(isdigit(*cptr))) {
    DEBUGP("ERROR snmpTargetAddrEntry: Row Status is not a digit in config string\n");
    return(0);
  }
  /* check that row status is a valid value */
  else if ( ((entry->rowStatus = (int)strtol(cptr, (char **)NULL, 0)) 
	     != SNMP_ROW_ACTIVE) &&
	    (entry->rowStatus != SNMP_ROW_NOTINSERVICE) &&
	    (entry->rowStatus != SNMP_ROW_NOTREADY) ) {
    sprintf(buff, "ERROR snmpTargetAddrEntry: Row Status is not a valid value of active(%d), notinservice(%d), or notready(%d) in config string.\n", SNMP_ROW_ACTIVE, SNMP_ROW_NOTINSERVICE, SNMP_ROW_NOTREADY);
    DEBUGP(buff);
    
    return(0);
  }
  return(1);
}  /* snmpTargetAddr_addRowStatus  */


void snmpd_parse_config_targetAddr(token, char_ptr)
     char *token, *char_ptr;
{
  char  *cptr = char_ptr, buff[1024];
  struct targetAddrTable_struct *newEntry;
  int                            i;

  newEntry = snmpTargetAddrTable_create();
  
  cptr = copy_word(cptr, buff);
  cptr = (char *)skip_white(cptr);
  if (snmpTargetAddr_addName(newEntry, buff) == 0) {
    snmpTargetAddrTable_dispose(newEntry);
    return;
  }
  cptr = copy_word(cptr, buff);
  cptr = (char *)skip_white(cptr);
  if (snmpTargetAddr_addTDomain(newEntry, buff) == 0) {
    snmpTargetAddrTable_dispose(newEntry);
    return;
  }
  cptr = copy_word(cptr, buff);
  cptr = (char *)skip_white(cptr);
  if (snmpTargetAddr_addTAddress(newEntry, buff) == 0) {
    snmpTargetAddrTable_dispose(newEntry);
    return;
  }
  cptr = copy_word(cptr, buff);
  cptr = (char *)skip_white(cptr);
  if (snmpTargetAddr_addTimeout(newEntry, buff) == 0) {
    snmpTargetAddrTable_dispose(newEntry);
    return;
  }
  cptr = copy_word(cptr, buff);
  cptr = (char *)skip_white(cptr);
  if (snmpTargetAddr_addRetryCount(newEntry, buff) == 0) {
    snmpTargetAddrTable_dispose(newEntry);
    return;
  }
  cptr = copy_word(cptr, buff);
  cptr = (char *)skip_white(cptr);
  if (snmpTargetAddr_addTagList(newEntry, buff) == 0) {
    snmpTargetAddrTable_dispose(newEntry);
    return;
  }
  cptr = copy_word(cptr, buff);
  cptr = (char *)skip_white(cptr);
  if (snmpTargetAddr_addParams(newEntry, buff) == 0) {
    snmpTargetAddrTable_dispose(newEntry);
    return;
  }
  cptr = copy_word(cptr, buff);
  cptr = (char *)skip_white(cptr);
  if (snmpTargetAddr_addStorageType(newEntry, buff) == 0) {
    snmpTargetAddrTable_dispose(newEntry);
    return;
  }
  cptr = copy_word(cptr, buff);
  cptr = (char *)skip_white(cptr);
  if (snmpTargetAddr_addRowStatus(newEntry, buff) == 0) {
    snmpTargetAddrTable_dispose(newEntry);
    return;
  }
  sprintf(buff, "snmp_parse_config_targetAddr, read: %s\n",
		   newEntry->name);
  for(i=0;i<newEntry->tDomainLen;i++) {
    sprintf(&buff[strlen(buff)], ".%d", (int)newEntry->tDomain[i]);
  }
  sprintf(&buff[strlen(buff)], " %s %d %d %s %s %d %d\n",
	  newEntry->tAddress, newEntry->timeout, newEntry->retryCount,
	  newEntry->tagList,  newEntry->params,  newEntry->storageType, 
	  newEntry->rowStatus);
  DEBUGP(buff);

  snmpTargetAddrTable_addToList(newEntry, &aAddrTable);
} /* snmpd_parse_config_target */


/* Shutdown routines */


/* shutdown_snmpTargetAddrEntry handles the shutdown proccess 
   for this MIB table. It writes out all the non-volatile rows 
   to permanent storage on a shutdown  */
void 
shutdown_snmpTargetAddrEntry(void)
{
  struct targetAddrTable_struct *curr_struct;
  char line[1024];
  int  i;

  if ( (curr_struct = aAddrTable) != 0) {
    while (curr_struct != 0) {
      if ( (curr_struct->storageType == SNMP_STORAGE_NONVOLATILE || 
	    curr_struct->storageType == SNMP_STORAGE_PERMANENT) 
	   &&
	   (curr_struct->rowStatus == SNMP_ROW_ACTIVE ||
	    curr_struct->rowStatus == SNMP_ROW_NOTINSERVICE) ) {
	sprintf(line, "targetAddr %s ", curr_struct->name);
	for(i=0; i < curr_struct->tDomainLen; i++) {
	  sprintf(&line[strlen(line)], ".%i", (int)curr_struct->tDomain[i]);
	}
	sprintf(&line[strlen(line)], " %s %i %i \"%s\" %s %i %i", 
			 curr_struct->tAddress,    curr_struct->timeout, 
			 curr_struct->retryCount,  curr_struct->tagList,
			 curr_struct->params,      curr_struct->storageType, 
			 curr_struct->rowStatus);
	
	/* store to file */
	snmpd_store_config(line);
      }
      curr_struct = curr_struct->next;
    }
  }

}  /*  shutdown_snmpTargetAddrEntry  */


/*MIB table access routines */


unsigned char *
var_snmpTargetAddrEntry(vp, name, length, exact, var_len, write_method)
    struct variable *vp;
    oid     *name;
    int     *length;
    int     exact;
    int     *var_len;
    int     (**write_method) (int, unsigned char *,unsigned 
			      char, int, unsigned char *,oid*, int);
{

  /* variables we may use later */
  static long                    long_ret;
  static unsigned char           string[1500];
  static oid                     objid[128];
  struct targetAddrTable_struct *temp_struct;
  int                            i=0;

  *write_method = 0;           /* assume it isnt writable for the time being */
  *var_len = sizeof(long_ret); /* assume an integer and change later if not */

  /* look for OID in current table */
  if ( (temp_struct = search_snmpTargetAddrTable(vp->name, (int)vp->namelen, 
				      name, length, exact)) == 0 ) {
    /* for creation of new rows */
    if (vp->magic == SNMPTARGETADDRROWSTATUS)  {
      *write_method = write_snmpTargetAddrRowStatus;
    }
    return(0);
  }
  
  /* We found what we were looking for, either the next OID or the exact OID */
  /* this is where we do the value assignments for the mib results. */
  switch(vp->magic) {

  case SNMPTARGETADDRTDOMAIN:
    *write_method = write_snmpTargetAddrTDomain;
    if (temp_struct->tDomainLen <= 0) {
      return(0);
    }
    else {
      for (i=0;i<temp_struct->tDomainLen;i++) {
	objid[i] = temp_struct->tDomain[i];
      }
      *var_len = temp_struct->tDomainLen * sizeof(oid);
    }
    return (unsigned char *) objid;
    
  case SNMPTARGETADDRTADDRESS:
    *write_method = write_snmpTargetAddrTAddress;
    if (temp_struct->tAddress == 0)  return(0);
    strcpy(string, temp_struct->tAddress);
    *var_len = strlen(string);
    return (unsigned char *) string;
    
  case SNMPTARGETADDRTIMEOUT:
    *write_method = write_snmpTargetAddrTimeout;
    long_ret = temp_struct->timeout;
    return (unsigned char *) &long_ret;

  case SNMPTARGETADDRRETRYCOUNT:
    *write_method = write_snmpTargetAddrRetryCount;
    long_ret = temp_struct->retryCount;
    return (unsigned char *) &long_ret;

  case SNMPTARGETADDRTAGLIST:
    *write_method = write_snmpTargetAddrTagList;
    strcpy(string, temp_struct->tagList);
    *var_len = strlen(string);
    return (unsigned char *) string;

  case SNMPTARGETADDRPARAMS:
    *write_method = write_snmpTargetAddrParams;
    if (temp_struct->params == 0) return(0);
    strcpy(string, temp_struct->params);
    *var_len = strlen(string);
    return (unsigned char *) string;
    
  case SNMPTARGETADDRSTORAGETYPE:
    *write_method = write_snmpTargetAddrStorageType;
    long_ret = temp_struct->storageType;
    return (unsigned char *) &long_ret;

  case SNMPTARGETADDRROWSTATUS:
    *write_method = write_snmpTargetAddrRowStatus;
    long_ret = temp_struct->rowStatus;
    return (unsigned char *) &long_ret;
    
  default:
    ERROR_MSG("ERROR: snmpTargetAddrEntry: magic not found.\n");
  }
  return 0;
}  /* var_snmpTargetAddrEntry */


int
write_snmpTargetAddrTDomain(action, var_val, var_val_type, 
			    var_val_len, statP, name, name_len)
   int      action;
   u_char   *var_val;
   u_char   var_val_type;
   int      var_val_len;
   u_char   *statP;
   oid      *name;
   int      name_len;
{
  int                            objSize=128, bigsize=1000, i;
  struct targetAddrTable_struct *temp_struct;
  static oid                     objid[128];

  if (var_val_type != ASN_OBJECT_ID) {
    DEBUGP("write to snmpTargetAddrTDomain not ASN_OBJECT_ID\n");
    return SNMP_ERR_WRONGTYPE;
  }
  if (var_val_len > 128) {
    DEBUGP("write to snmpTargetAddrTDomain: bad length\n");
    return SNMP_ERR_WRONGLENGTH;
  }

  /* spec check, ??? */
  asn_parse_objid(var_val, &bigsize, &var_val_type, objid, &objSize);
  
  snmpTargetAddrOID[snmpTargetAddrOIDLen-1] = SNMPTARGETADDRTDOMAINCOLUMN;
  if ( (temp_struct = search_snmpTargetAddrTable(snmpTargetAddrOID, 
						 snmpTargetAddrOIDLen,
						 name, &name_len, 1)) == 0) {
    DEBUGP("write to snmpTargetAddrTDomain : BAD OID!\n");
    return SNMP_ERR_NOSUCHNAME;
  }
  /* row exists, check if it is changeable */
  if (temp_struct->storageType == SNMP_STORAGE_READONLY) {
    DEBUGP("write to snmpTargetAddrTDomain : row is read only\n");
    return SNMP_ERR_READONLY;
  }
  /* check if row active */
  if (temp_struct->rowStatus == SNMP_ROW_ACTIVE) {
    DEBUGP("write to snmpTargetAddrTDomains : This change not allowed in active row.\n");
    return SNMP_ERR_INCONSISTENTVALUE;
  }

  /* Finally, we're golden, check if we should save value */
  if (action == COMMIT)  {    
    for (i=0;i<objSize;i++) 
      temp_struct->tDomain[i] = objid[i];
    temp_struct->tDomainLen = objSize;

    /* If row is new, check if its status can be updated */
    if ( (temp_struct->rowStatus == SNMP_ROW_NOTREADY) &&
	 (snmpTargetAddr_rowStatusCheck(temp_struct) != 0) )
      temp_struct->rowStatus = SNMP_ROW_NOTINSERVICE;
  }

  return SNMP_ERR_NOERROR;
}  /* write_snmpTargetAddrTDomain */


int
write_snmpTargetAddrTAddress(action, var_val, var_val_type, var_val_len, 
			     statP, name, name_len)
   int      action;
   u_char   *var_val;
   u_char   var_val_type;
   int      var_val_len;
   u_char   *statP;
   oid      *name;
   int      name_len;
{
  static unsigned char           string[1500];
  int                            size=1500, bigsize=1000;
  struct targetAddrTable_struct *temp_struct;

  if (var_val_type != ASN_OCTET_STR) {
      DEBUGP("write to snmpTargetAddrTAddress not ASN_OCTET_STR\n");
      return SNMP_ERR_WRONGTYPE;
  }
  if (var_val_len > (size = sizeof(string))) {
      DEBUGP("write to snmpTargetAddrTAddress: bad length\n");
      return SNMP_ERR_WRONGLENGTH;
  }

  /* spec check, ??? */
  asn_parse_string(var_val, &bigsize, &var_val_type, string, &size);
  
  snmpTargetAddrOID[snmpTargetAddrOIDLen-1] = SNMPTARGETADDRTADDRESSCOLUMN;
  if ( (temp_struct = search_snmpTargetAddrTable(snmpTargetAddrOID, 
						 snmpTargetAddrOIDLen,
						 name, &name_len, 1)) == 0) {
    DEBUGP("write to snmpTargetAddrTAddress : BAD OID!\n");
    return SNMP_ERR_NOSUCHNAME;
  }
  /* row exists, check if it is changeable */
  if (temp_struct->storageType == SNMP_STORAGE_READONLY) {
    DEBUGP("write to snmpTargetAddrTAddress : row is read only\n");
    return SNMP_ERR_READONLY;
  }
  /* check if row active */
  if (temp_struct->rowStatus == SNMP_ROW_ACTIVE) {
    DEBUGP("write to snmpTargetAddrTAddress : This change not allowed in active row.\n");
    return SNMP_ERR_INCONSISTENTVALUE;
  }
  
  /* Finally, we're golden, check if we should save value */
  if (action == COMMIT)  {    
    free(temp_struct->tAddress);
    temp_struct->tAddress = (char *)malloc( (size*sizeof(char))+1 );
    memcpy(temp_struct->tAddress, string, size*sizeof(char));
    temp_struct->tAddress[size] = '\0';
    
    /* If row is new, check if its status can be updated */
    if ( (temp_struct->rowStatus == SNMP_ROW_NOTREADY) &&
	 (snmpTargetAddr_rowStatusCheck(temp_struct) != 0) )
      temp_struct->rowStatus = SNMP_ROW_NOTINSERVICE;
  }
  return SNMP_ERR_NOERROR;
}  /* write_snmpTargetAddrTAddress */


int
write_snmpTargetAddrTimeout(action, var_val, var_val_type, var_val_len, 
			    statP, name, name_len)
   int      action;
   u_char   *var_val;
   u_char   var_val_type;
   int      var_val_len;
   u_char   *statP;
   oid      *name;
   int      name_len;
{
  /* variables we may use later */
  static long                    long_ret;
  int                            size, bigsize=1000;
  struct targetAddrTable_struct *temp_struct;

  if (var_val_type != ASN_INTEGER) {
      DEBUGP("write to snmpTargetAddrTimeout not ASN_INTEGER\n");
      return SNMP_ERR_WRONGTYPE;
  }
  if (var_val_len > (size = sizeof(long_ret))) {
      DEBUGP("write to snmpTargetAddrTimeout: bad length\n");
      return SNMP_ERR_WRONGLENGTH;
  }
  size = sizeof(long_ret);
  asn_parse_int(var_val, &bigsize, &var_val_type, &long_ret, size);
  
  /* spec check range, no spec check */
  
  /* Find row in linked list and check pertinent status... */
  snmpTargetAddrOID[snmpTargetAddrOIDLen-1] = SNMPTARGETADDRTIMEOUTCOLUMN;
  if ((temp_struct = search_snmpTargetAddrTable
       (snmpTargetAddrOID, snmpTargetAddrOIDLen, 
	name, &name_len, 1)) == 0 ) {
    DEBUGP("write to snmpTargetAddrTimeout : BAD OID\n");
    return SNMP_ERR_NOSUCHNAME;
  }
  /* row exists, check if it is changeable */
  if (temp_struct->storageType == SNMP_STORAGE_READONLY) {
    DEBUGP("write to snmpTargetAddrTimeout : row is read only\n");
    return SNMP_ERR_READONLY;
  }

  /* Finally, we're golden, should we save value? */
  if (action == COMMIT)  {
    temp_struct->timeout = long_ret;
  }

  return SNMP_ERR_NOERROR;
}  /* write_snmpTargetAddrTimeout */


int
write_snmpTargetAddrRetryCount(action, var_val, var_val_type, var_val_len, 
			       statP, name, name_len)
   int      action;
   u_char   *var_val;
   u_char   var_val_type;
   int      var_val_len;
   u_char   *statP;
   oid      *name;
   int      name_len;
{
  /* variables we may use later */
  static long                    long_ret;
  int                            size, bigsize=1000;
  struct targetAddrTable_struct *temp_struct;

  if (var_val_type != ASN_INTEGER) {
    DEBUGP("write to snmpTargetAddrRetryCount not ASN_INTEGER\n");
    return SNMP_ERR_WRONGTYPE;
  }
  if (var_val_len > (size = sizeof(long_ret))) {
    DEBUGP("write to snmpTargetAddrRetryCount: bad length\n");
    return SNMP_ERR_WRONGLENGTH;
  }
  size = sizeof(long_ret);
  asn_parse_int(var_val, &bigsize, &var_val_type, &long_ret, size);
  
  /* spec check range, no spec check */
  
  /* Find row in linked list and check pertinent status... */
  snmpTargetAddrOID[snmpTargetAddrOIDLen-1] = SNMPTARGETADDRRETRYCOUNTCOLUMN;
  if ((temp_struct = search_snmpTargetAddrTable
       (snmpTargetAddrOID, snmpTargetAddrOIDLen, 
	name, &name_len, 1)) == 0 ) {
    DEBUGP("write to snmpTargetAddrTimeout : BAD OID\n");
    return SNMP_ERR_NOSUCHNAME;
  }

  if (temp_struct->storageType == SNMP_STORAGE_READONLY) {
    DEBUGP("write to snmpTargetAddrRetryCount : row is read only\n");
    return SNMP_ERR_READONLY;
  }

  /* Finally, we're golden, should we save value? */
  if (action == COMMIT)  {
    temp_struct->retryCount = long_ret;
  }

  return SNMP_ERR_NOERROR;
}  /* write_snmpTargetAddrRetryCount */


int
write_snmpTargetAddrTagList(action, var_val, var_val_type, var_val_len, 
			    statP, name, name_len)
   int      action;
   u_char   *var_val;
   u_char   var_val_type;
   int      var_val_len;
   u_char   *statP;
   oid      *name;
   int      name_len;
{
  /* variables we may use later */
  static unsigned char           string[1500];
  int                            size=1500, bigsize=1000;
  struct targetAddrTable_struct *temp_struct;

  if (var_val_type != ASN_OCTET_STR) {
    DEBUGP("write to snmpTargetAddrTagList not ASN_OCTET_STR\n");
    return SNMP_ERR_WRONGTYPE;
  }
  if (var_val_len > (size = sizeof(string))) {
    DEBUGP("write to snmpTargetAddrTagList: bad length\n");
    return SNMP_ERR_WRONGLENGTH;
  }
  
  /* spec check, ??? */
  asn_parse_string(var_val, &bigsize, &var_val_type, string, &size);
  
  snmpTargetAddrOID[snmpTargetAddrOIDLen-1] = SNMPTARGETADDRTAGLISTCOLUMN;
  if ( (temp_struct = search_snmpTargetAddrTable(snmpTargetAddrOID, 
						 snmpTargetAddrOIDLen,
						 name, &name_len, 1)) == 0) {
    DEBUGP("write to snmpTargetAddrTagList : BAD OID!\n");
    return SNMP_ERR_NOSUCHNAME;
  }
  /* row exists, check if it is changeable */
  if (temp_struct->storageType == SNMP_STORAGE_READONLY) {
    DEBUGP("write to snmpTargetAddrTagList : row is read only\n");
    return SNMP_ERR_READONLY;
  }
  
  /* Finally, we're golden, check if we should save value */
  if (action == COMMIT)  {    
    free(temp_struct->tagList);
    temp_struct->tagList = (char *)malloc( (size*sizeof(char))+1 );
    memcpy(temp_struct->tagList, string, size*sizeof(char));
    temp_struct->tagList[size] = '\0';
  }

  return SNMP_ERR_NOERROR;
}  /* write_snmpTargetAddrTagList */


int
write_snmpTargetAddrParams(action, var_val, var_val_type, var_val_len, 
			   statP, name, name_len)
   int      action;
   u_char   *var_val;
   u_char   var_val_type;
   int      var_val_len;
   u_char   *statP;
   oid      *name;
   int      name_len;
{
  static unsigned char           string[1500];
  int                            size=1500, bigsize=1000;
  struct targetAddrTable_struct *temp_struct;

  if (var_val_type != ASN_OCTET_STR) {
    DEBUGP("write to snmpTargetAddrParams not ASN_OCTET_STR\n");
    return SNMP_ERR_WRONGTYPE;
  }
  if (var_val_len > (size = sizeof(string))) {
    DEBUGP("write to snmpTargetAddrParams: bad length\n");
    return SNMP_ERR_WRONGLENGTH;
  }

  /* spec check, ??? */
  asn_parse_string(var_val, &bigsize, &var_val_type, string, &size);
  
  snmpTargetAddrOID[snmpTargetAddrOIDLen-1] = SNMPTARGETADDRPARAMSCOLUMN;
  if ( (temp_struct = search_snmpTargetAddrTable(snmpTargetAddrOID, 
						 snmpTargetAddrOIDLen,
						 name, &name_len, 1)) == 0) {
    DEBUGP("write to snmpTargetAddrParams : BAD OID!\n");
    return SNMP_ERR_NOSUCHNAME;
  }
  /* row exists, check if it is changeable */
  if (temp_struct->storageType == SNMP_STORAGE_READONLY) {
    DEBUGP("write to snmpTargetAddrParams : row is read only\n");
    return SNMP_ERR_READONLY;
  }
  
  /* Finally, we're golden, check if we should save value */
  if (action == COMMIT)  {    
    free(temp_struct->params);
    temp_struct->params = (char *)malloc( (size*sizeof(char))+1 );
    memcpy(temp_struct->params, string, size*sizeof(char));
    temp_struct->params[size] = '\0';
    
    /* If row is new, check if its status can be updated */
    if ( (temp_struct->rowStatus == SNMP_ROW_NOTREADY) &&
	 (snmpTargetAddr_rowStatusCheck(temp_struct) != 0) )
      temp_struct->rowStatus = SNMP_ROW_NOTINSERVICE;
  }

  return SNMP_ERR_NOERROR;
}  /* write_snmpTargetAddrParams */


int
write_snmpTargetAddrStorageType(action, var_val, var_val_type, var_val_len, 
				statP, name, name_len)
   int      action;
   u_char   *var_val;
   u_char   var_val_type;
   int      var_val_len;
   u_char   *statP;
   oid      *name;
   int      name_len;
{
  static long                    long_ret;
  int                            size, bigsize=1000;
  struct targetAddrTable_struct *temp_struct;

  if (var_val_type != ASN_INTEGER) {
      DEBUGP("write to snmpTargetAddrStorageType not ASN_INTEGER\n");
      return SNMP_ERR_WRONGTYPE;
  }
  if (var_val_len > (size = sizeof(long_ret))) {
      DEBUGP("write to snmpTargetAddrStorageType: bad length\n");
      return SNMP_ERR_WRONGLENGTH;
  }

  size = sizeof(long_ret);
  asn_parse_int(var_val, &bigsize, &var_val_type, &long_ret, size);
  
  if ( (long_ret != SNMP_STORAGE_OTHER) && (long_ret != SNMP_STORAGE_VOLATILE) &&
       (long_ret != SNMP_STORAGE_NONVOLATILE) )  {
    DEBUGP("write to snmpTargetAddrStorageType : attempted storage type not a valid");
    DEBUGP("  value of other(%d), volatile(%d), or nonvolatile(%d)\n", 
	   SNMP_STORAGE_OTHER, SNMP_STORAGE_VOLATILE, SNMP_STORAGE_NONVOLATILE);
    return SNMP_ERR_INCONSISTENTVALUE;
  }

  /* Find the struct in the linked list and check status */
  snmpTargetAddrOID[snmpTargetAddrOIDLen-1] = SNMPTARGETADDRSTORAGETYPECOLUMN;
  if ((temp_struct = search_snmpTargetAddrTable(snmpTargetAddrOID, 
						snmpTargetAddrOIDLen, 
						name, &name_len, 1)) == 0 ) {
    DEBUGP("write to snmpTargetAddrStorageType : BAD OID\n");
    return SNMP_ERR_NOSUCHNAME;
  }
  if ( (temp_struct->storageType == SNMP_STORAGE_PERMANENT) || 
       (temp_struct->storageType == SNMP_STORAGE_READONLY) )  {
    DEBUGP("write to snmpTargetAddrStorageType : row has unchangeable storage status: %d\n",
	   temp_struct->storageType);
    return SNMP_ERR_INCONSISTENTVALUE;
  }

  /* Finally, we're golden, check if we should save new value */
  if (action == COMMIT) {      
    temp_struct->storageType = long_ret;
  }

  return SNMP_ERR_NOERROR;
}  /* write_snmpTargetAddrStorageType */


/* snmpTargeAddr_createNewRow is called from write_snmpTargetAddrRowStatus
   when a new row is required. It creates a new row with 
   the index of the passed in 'name' (i.e. full index OID) and
   adds it to the linked list. 'name' should be the full OID of the new index. 
   It passes back 0 if unsuccessfull.*/
int snmpTargetAddr_createNewRow(
     oid  *name,
     int  name_len)
{
  int    newNameLen, i;
  struct targetAddrTable_struct *temp_struct;

  /* setup a new snmpTargetAddrTable structure and add it to the list */
  newNameLen = name_len - snmpTargetAddrOIDLen;
  if (newNameLen > 0) {
    temp_struct       = snmpTargetAddrTable_create();
    temp_struct->name = (char *)malloc(sizeof(char)*(newNameLen + 1));

    for (i = 0; i < newNameLen; i++) {
      temp_struct->name[i] = (char)name[i+snmpTargetAddrOIDLen];
    }

    temp_struct->name[newNameLen]  = '\0';
    temp_struct->rowStatus         = SNMP_ROW_NOTREADY;
    
    snmpTargetAddrTable_addToList(temp_struct, &aAddrTable);

    return 1;
  }

  return 0;
}  /* snmpTargetAddr_createNewRow */
	

/* Assign a value to the Row Status variable */
int
write_snmpTargetAddrRowStatus(action, var_val, var_val_type, var_val_len, 
			      statP, name, name_len)
   int      action;
   u_char   *var_val;
   u_char   var_val_type;
   int      var_val_len;
   u_char   *statP;
   oid      *name;
   int      name_len;
{
  enum commit_action_enum        {NOTHING, DESTROY, CREATE, CHANGE};
  enum commit_action_enum        onCommitDo = NOTHING; 
  static long                    long_ret;
  int                            size, bigsize=1000;
  struct targetAddrTable_struct *temp_struct;

  if (var_val_type != ASN_INTEGER) {
      DEBUGP("write to snmpTargetAddrRowStatus not ASN_INTEGER\n");
      return SNMP_ERR_WRONGTYPE;
  }
  if (var_val_len > (size = sizeof(long_ret))) {
      DEBUGP("write to snmpTargetAddrRowStatus: bad length\n");
      return SNMP_ERR_WRONGLENGTH;
  }
  size = sizeof(long_ret);
  asn_parse_int(var_val, &bigsize, &var_val_type, &long_ret, size);

  /* search for struct in linked list */
  snmpTargetAddrOID[snmpTargetAddrOIDLen-1] = SNMPTARGETADDRROWSTATUSCOLUMN;
  if ((temp_struct = search_snmpTargetAddrTable(snmpTargetAddrOID, 
						snmpTargetAddrOIDLen, 
						name, &name_len, 1)) == 0) {
    /* row doesn't exist, check valid possibilities */
    if (long_ret == SNMP_ROW_DESTROY)  
      /* re: RFC 1903, destroying a non-existent row is noError, whatever */
      onCommitDo = NOTHING;
    /* check if this is for a new row creation */
    else if (long_ret == SNMP_ROW_CREATEANDGO || long_ret == SNMP_ROW_CREATEANDWAIT) 
      onCommitDo = CREATE;
    else /* no valid sets for no row being found so... */
      return SNMP_ERR_NOSUCHNAME;
  }
  else {  /* row exists */

    /* check if it is changeable */
    if (temp_struct->storageType == SNMP_STORAGE_READONLY) {
      DEBUGP("write to snmpTargetAddrRowStatus : row is read only\n");
      return SNMP_ERR_READONLY;
    }    
    /* check if row is to be destroyed (note: it is ok to destroy notReady row!) */
    else if (long_ret == SNMP_ROW_DESTROY)  {
      if (temp_struct->storageType == SNMP_STORAGE_PERMANENT) {
	DEBUGP("write to snmpTargetAddrRowStatus : unable to destroy permanent row\n");
	return SNMP_ERR_INCONSISTENTVALUE;
      }
      else  {
	onCommitDo = DESTROY;
      }
    }
    /* check if row is new and can be changed from notready yet */
    else if (temp_struct->rowStatus == SNMP_ROW_NOTREADY) {
      DEBUGP("write to snmpTargeAddrRowStatus : unable to change from NOTREADY\n");
      return SNMP_ERR_INCONSISTENTVALUE;
    }  
    /* we now know the row status can be set, check for the two valid settings left*/
    else if ( (long_ret == SNMP_ROW_ACTIVE) || 
	      (long_ret == SNMP_ROW_NOTINSERVICE) ) {
      onCommitDo = CHANGE;
    }
    /* not a valid setting */
    else  {
      DEBUGP("write to snmpTargetAddrRowStatus : Bad value for set\n");
      return SNMP_ERR_INCONSISTENTVALUE;
    }
  } /* if row exist */
  

  /* if this is a commit, do expected action */
  if (action == COMMIT) {
    switch(onCommitDo) { 
      
    case CREATE :
      if (snmpTargetAddr_createNewRow(name, name_len) == 0) {
	DEBUGP("write to snmpTargetAddrRowStatus : ");
	DEBUGP("failed new row creation, bad OID/index value \n");
	return SNMP_ERR_GENERR;
      }
      break;
      
    case DESTROY:
      snmpTargetAddrTable_remFromList(temp_struct, &aAddrTable);
      break;

    case CHANGE:
      temp_struct->rowStatus = long_ret;
      break;

    case NOTHING:
      break;
    }
  }
  
  return SNMP_ERR_NOERROR;
}  /* write_snmpTargetAddrRowStatus */

