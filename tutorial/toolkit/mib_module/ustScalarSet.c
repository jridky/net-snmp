/* Most of this file was generated by the mib2c perl script. */

#ifdef IN_UCD_SNMP_SOURCE
/* If we're compiling this file inside the ucd-snmp source tree */

/* This should always be included first before anything else */
#include <config.h>

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

/* minimal include directives */
#include "mibincl.h"
#include "util_funcs.h"

#else /* !IN_UCD_SNMP_SOURCE */

#include <ucd-snmp/ucd-snmp-config.h>
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#include <ucd-snmp/ucd-snmp-includes.h>
#include <ucd-snmp/ucd-snmp-agent-includes.h>

#endif /* !IN_UCD_SNMP_SOURCE */

#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include "ustScalarSet.h"

/* 
 * ustScalarSet_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */

static oid ustScalarSet_variables_oid[] = { 1,3,6,1,4,1,2021,13,4242,1,1 };

/*
 * Global variables to store data we're interesting in serving:
 */
static char   *ustSSSimpleString;
static size_t  ustSSSimpleString_len;
static time_t  lastChanged=0;


/* 
 * variable2 ustScalarSet_variables:
 *   this variable defines function callbacks and type return information 
 *   for the ustScalarSet mib section 
 */


struct variable2 ustScalarSet_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
#define   USTSSSIMPLESTRING     1
  { USTSSSIMPLESTRING   , ASN_OCTET_STR , RWRITE, var_ustScalarSet, 1, { 1 } },
#define   USTSSSECONDSSINCECHANGED  2
  { USTSSSECONDSSINCECHANGED, ASN_TIMETICKS , RONLY , var_ustScalarSet, 1, { 2 } },

};
/*    (L = length of the oidsuffix) */

/* deinit call for supporting dynamic shared object loading/unloading */
void deinit_ustScalarSet(void) {
    DEBUGMSGTL(("ustScalarSet","unloading\n"));
    unregister_mib(ustScalarSet_variables_oid,
                   sizeof(ustScalarSet_variables_oid)/sizeof(oid));
}

/*
 * init_ustScalarSet():
 *   Initialization routine.  This is called when the agent starts up.
 *   At a minimum, registration of your variables should take place here.
 */
void init_ustScalarSet(void) {
  DEBUGMSGTL(("ustScalarSet","initializing\n"));

  /* register ourselves with the agent to handle our mib tree */
  REGISTER_MIB("ustScalarSet", ustScalarSet_variables, variable2,
               ustScalarSet_variables_oid);


  /* place any other initialization junk you need here */
  ustSSSimpleString = strdup("Hello World");
  ustSSSimpleString_len = strlen(ustSSSimpleString);
  lastChanged = time(NULL);
}


/*
 * var_ustScalarSet():
 *   This function is called every time the agent gets a request for
 *   a scalar variable that might be found within your mib section
 *   registered above.  It is up to you to do the right thing and
 *   return the correct value.
 *     You should also correct the value of "var_len" if necessary.
 *
 *   Please see the documentation for more information about writing
 *   module extensions, and check out the examples in the examples
 *   and mibII directories.
 */
unsigned char *
var_ustScalarSet(struct variable *vp, 
                oid     *name, 
                size_t  *length, 
                int     exact, 
                size_t  *var_len, 
                WriteMethod **write_method)
{
  /* variables we may use later */
  static long long_ret;

  if (header_generic(vp,name,length,exact,var_len,write_method)
                                  == MATCH_FAILED )
    return NULL;

  /* 
   * this is where we do the value assignments for the mib results.
   */
  switch(vp->magic) {

    case USTSSSIMPLESTRING:
        *write_method = write_ustSSSimpleString;
        *var_len = ustSSSimpleString_len;
        return (unsigned char *) ustSSSimpleString;

    case USTSSSECONDSSINCECHANGED:
        /* TimeTicks are seconds*100 */
        long_ret = (time(NULL) - lastChanged)*100;
        *var_len = sizeof(long_ret);
        return (unsigned char *) &long_ret;

    default:
      ERROR_MSG("");
  }
  return NULL;
}


int
write_ustSSSimpleString(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static unsigned char *new_string = 0, *old_string = 0;
  static size_t size, old_size;

 /* this long complex series of "action"s is to preserve proper
    transaction handling with other transactions in the same set
    request. */

  switch ( action ) {
        case RESERVE1:
          /* check to see that everything is possible */
          if (var_val_type != ASN_OCTET_STR){
              fprintf(stderr, "write to ustSSSimpleString not ASN_OCTET_STR\n");
              return SNMP_ERR_WRONGTYPE;
          }
          break;


        case RESERVE2:
          /* allocate memory needed here. */
          size = var_val_len;
          new_string = (char *) malloc(size+1);
          if (new_string == NULL) {
            return SNMP_ERR_GENERR; /* malloc failed! */
          }
          break;



        case ACTION:
             /* Actually make the change requested.  Note that
             anything done here must be reversable in the UNDO case */
          if (new_string) {
            old_string = ustSSSimpleString;
            old_size = ustSSSimpleString_len;
            memcpy(new_string, var_val, var_val_len);
            new_string[var_val_len] = 0;
            ustSSSimpleString = new_string;
            ustSSSimpleString_len = size;
            new_string = NULL;
          } else {
            /* something seriously wrong if we got here */
            return SNMP_ERR_GENERR;
          }
          break;


        case UNDO:
          /* Back out any changes made in the ACTION case */
          if (old_string == NULL) {
            return SNMP_ERR_UNDOFAILED;
          }
          if (ustSSSimpleString)
            free(ustSSSimpleString);
          ustSSSimpleString = old_string;
          ustSSSimpleString_len = old_size;
          break;


        case COMMIT:
          /* Things are working well, so it's now safe to make the change
             permanently.  Make sure that anything done here can't fail! */
          lastChanged = time(NULL);
	  if (old_string)
	    free(old_string);
          break;
          
          /* Treat the rest the same as FREE */
        case FREE:
          break;
          /* Release any resources that have been allocated */
          if (new_string) {
            free(new_string);
            new_string = NULL;
          }
          if (old_string) {
            free(old_string);
            old_string = NULL;
          }
          break;

  }
  return SNMP_ERR_NOERROR;
}
