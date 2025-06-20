/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

/* src/include/mpi.h.  Generated from mpi.h.in by configure. */
#ifndef MPI_INCLUDED
#define MPI_INCLUDED

/* user include file for MPI programs */

#if defined(HAVE_VISIBILITY)
#define MPICH_API_PUBLIC __attribute__((visibility ("default")))
#else
#define MPICH_API_PUBLIC
#endif

#define MPI_VERSION    4
#define MPI_SUBVERSION 1
#define MPICH_NAME     4
#define MPICH         1
#define MPICH_HAS_C2F  1

/* MPICH_VERSION is the version string. MPICH_NUMVERSION is the
 * numeric version that can be used in numeric comparisons.
 *
 * MPICH_VERSION uses the following format:
 * Version: [MAJ].[MIN].[REV][EXT][EXT_NUMBER]
 * Example: 1.0.7rc1 has
 *          MAJ = 1
 *          MIN = 0
 *          REV = 7
 *          EXT = rc
 *          EXT_NUMBER = 1
 *
 * MPICH_NUMVERSION will convert EXT to a format number:
 *          ALPHA (a) = 0
 *          BETA (b)  = 1
 *          RC (rc)   = 2
 *          PATCH (p) = 3
 * Regular releases are treated as patch 0
 *
 * Numeric version will have 1 digit for MAJ, 2 digits for MIN, 2
 * digits for REV, 1 digit for EXT and 2 digits for EXT_NUMBER. So,
 * 1.0.7rc1 will have the numeric version 10007201.
 */
#define MPICH_VERSION "4.2.1"
#define MPICH_NUMVERSION 40201300

#define MPICH_RELEASE_TYPE_ALPHA  0
#define MPICH_RELEASE_TYPE_BETA   1
#define MPICH_RELEASE_TYPE_RC     2
#define MPICH_RELEASE_TYPE_PATCH  3

#define MPICH_CALC_VERSION(MAJOR, MINOR, REVISION, TYPE, PATCH) \
    (((MAJOR) * 10000000) + ((MINOR) * 100000) + ((REVISION) * 1000) + ((TYPE) * 100) + (PATCH))

/* Keep C++ compilers from getting confused */
#if defined(__cplusplus)
extern "C" {
#endif

#define NO_TAGS_WITH_MODIFIERS 1
#undef MPICH_DEFINE_ATTR_TYPE_TYPES
#if defined(__has_attribute)
#  if __has_attribute(pointer_with_type_tag) && \
      __has_attribute(type_tag_for_datatype) && \
      !defined(NO_TAGS_WITH_MODIFIERS) &&\
      !defined(MPICH_NO_ATTR_TYPE_TAGS)
#    define MPICH_DEFINE_ATTR_TYPE_TYPES 1
#    define MPICH_ATTR_POINTER_WITH_TYPE_TAG(buffer_idx, type_idx)  __attribute__((pointer_with_type_tag(MPI,buffer_idx,type_idx)))
#    define MPICH_ATTR_TYPE_TAG(type)                               __attribute__((type_tag_for_datatype(MPI,type)))
#    define MPICH_ATTR_TYPE_TAG_LAYOUT_COMPATIBLE(type)             __attribute__((type_tag_for_datatype(MPI,type,layout_compatible)))
#    define MPICH_ATTR_TYPE_TAG_MUST_BE_NULL()                      __attribute__((type_tag_for_datatype(MPI,void,must_be_null)))
#    include <stddef.h>
#  endif
#endif

#if !defined(MPICH_ATTR_POINTER_WITH_TYPE_TAG)
#  define MPICH_ATTR_POINTER_WITH_TYPE_TAG(buffer_idx, type_idx)
#  define MPICH_ATTR_TYPE_TAG(type)
#  define MPICH_ATTR_TYPE_TAG_LAYOUT_COMPATIBLE(type)
#  define MPICH_ATTR_TYPE_TAG_MUST_BE_NULL()
#endif

#if !defined(INT8_C)
/* stdint.h was not included, see if we can get it */
#  if defined(__cplusplus)
#    if __cplusplus >= 201103
#      include <cstdint>
#    endif
#  endif
#endif

#if !defined(INT8_C)
/* stdint.h was not included, see if we can get it */
#  if defined(__STDC_VERSION__)
#    if __STDC_VERSION__ >= 199901
#      include <stdint.h>
#    endif
#  endif
#endif

#if defined(INT8_C)
/* stdint.h was included, so we can annotate these types */
#  define MPICH_ATTR_TYPE_TAG_STDINT(type) MPICH_ATTR_TYPE_TAG(type)
#else
#  define MPICH_ATTR_TYPE_TAG_STDINT(type)
#endif

#ifdef __STDC_VERSION__
#if __STDC_VERSION__ >= 199901
#  define MPICH_ATTR_TYPE_TAG_C99(type) MPICH_ATTR_TYPE_TAG(type)
#else
#  define MPICH_ATTR_TYPE_TAG_C99(type)
#endif
#else
#  define MPICH_ATTR_TYPE_TAG_C99(type)
#endif

#if defined(__cplusplus)
#  define MPICH_ATTR_TYPE_TAG_CXX(type) MPICH_ATTR_TYPE_TAG(type)
#else
#  define MPICH_ATTR_TYPE_TAG_CXX(type)
#endif

#define MPIU_DLL_SPEC

/* Define some null objects */
#define MPI_COMM_NULL      ((MPI_Comm)0x04000000)
#define MPI_OP_NULL        ((MPI_Op)0x18000000)
#define MPI_GROUP_NULL     ((MPI_Group)0x08000000)
#define MPI_DATATYPE_NULL  ((MPI_Datatype)0x0c000000)
#define MPI_REQUEST_NULL   ((MPI_Request)0x2c000000)
#define MPI_ERRHANDLER_NULL ((MPI_Errhandler)0x14000000)
#define MPI_MESSAGE_NULL   ((MPI_Message)0x2c000000)
#define MPI_MESSAGE_NO_PROC ((MPI_Message)0x6c000000)
#define MPIX_STREAM_NULL   ((MPIX_Stream)0x3c000000)

typedef int MPI_Datatype;
#define MPI_CHAR           ((MPI_Datatype)0x4c000101)
#define MPI_SIGNED_CHAR    ((MPI_Datatype)0x4c000118)
#define MPI_UNSIGNED_CHAR  ((MPI_Datatype)0x4c000102)
#define MPI_BYTE           ((MPI_Datatype)0x4c00010d)
#define MPI_WCHAR          ((MPI_Datatype)0x4c00040e)
#define MPI_SHORT          ((MPI_Datatype)0x4c000203)
#define MPI_UNSIGNED_SHORT ((MPI_Datatype)0x4c000204)
#define MPI_INT            ((MPI_Datatype)0x4c000405)
#define MPI_UNSIGNED       ((MPI_Datatype)0x4c000406)
#define MPI_LONG           ((MPI_Datatype)0x4c000807)
#define MPI_UNSIGNED_LONG  ((MPI_Datatype)0x4c000808)
#define MPI_FLOAT          ((MPI_Datatype)0x4c00040a)
#define MPI_DOUBLE         ((MPI_Datatype)0x4c00080b)
#define MPI_LONG_DOUBLE    ((MPI_Datatype)0x4c00100c)
#define MPI_LONG_LONG_INT  ((MPI_Datatype)0x4c000809)
#define MPI_UNSIGNED_LONG_LONG ((MPI_Datatype)0x4c000819)
#define MPI_LONG_LONG      MPI_LONG_LONG_INT

#define MPI_PACKED         ((MPI_Datatype)0x4c00010f)
#define MPI_LB             ((MPI_Datatype)0x4c000010)
#define MPI_UB             ((MPI_Datatype)0x4c000011)

/*
   The layouts for the types MPI_DOUBLE_INT etc are simply
   struct {
       double var;
       int    loc;
   }
   This is documented in the man pages on the various datatypes.
 */
#define MPI_FLOAT_INT         ((MPI_Datatype)0x8c000000)
#define MPI_DOUBLE_INT        ((MPI_Datatype)0x8c000001)
#define MPI_LONG_INT          ((MPI_Datatype)0x8c000002)
#define MPI_SHORT_INT         ((MPI_Datatype)0x8c000003)
#define MPI_2INT              ((MPI_Datatype)0x4c000816)
#define MPI_LONG_DOUBLE_INT   ((MPI_Datatype)0x8c000004)

/* Fortran types */
#define MPI_COMPLEX           ((MPI_Datatype)0x4c00081e)
#define MPI_DOUBLE_COMPLEX    ((MPI_Datatype)0x4c001022)
#define MPI_LOGICAL           ((MPI_Datatype)0x4c00041d)
#define MPI_REAL              ((MPI_Datatype)0x4c00041c)
#define MPI_DOUBLE_PRECISION  ((MPI_Datatype)0x4c00081f)
#define MPI_INTEGER           ((MPI_Datatype)0x4c00041b)
#define MPI_2INTEGER          ((MPI_Datatype)0x4c000820)
#define MPI_2REAL             ((MPI_Datatype)0x4c000821)
#define MPI_2DOUBLE_PRECISION ((MPI_Datatype)0x4c001023)
#define MPI_CHARACTER         ((MPI_Datatype)0x4c00011a)

/* Size-specific types (see MPI-2, 10.2.5) */
#define MPI_REAL4             ((MPI_Datatype)0x4c000427)
#define MPI_REAL8             ((MPI_Datatype)0x4c000829)
#define MPI_REAL16            ((MPI_Datatype)0x4c00102b)
#define MPI_COMPLEX8          ((MPI_Datatype)0x4c000828)
#define MPI_COMPLEX16         ((MPI_Datatype)0x4c00102a)
#define MPI_COMPLEX32         ((MPI_Datatype)0x4c00202c)
#define MPI_INTEGER1          ((MPI_Datatype)0x4c00012d)
#define MPI_INTEGER2          ((MPI_Datatype)0x4c00022f)
#define MPI_INTEGER4          ((MPI_Datatype)0x4c000430)
#define MPI_INTEGER8          ((MPI_Datatype)0x4c000831)
#define MPI_INTEGER16         ((MPI_Datatype)MPI_DATATYPE_NULL)

/* C99 fixed-width datatypes */
#define MPI_INT8_T            ((MPI_Datatype)0x4c000137)
#define MPI_INT16_T           ((MPI_Datatype)0x4c000238)
#define MPI_INT32_T           ((MPI_Datatype)0x4c000439)
#define MPI_INT64_T           ((MPI_Datatype)0x4c00083a)
#define MPI_UINT8_T           ((MPI_Datatype)0x4c00013b)
#define MPI_UINT16_T          ((MPI_Datatype)0x4c00023c)
#define MPI_UINT32_T          ((MPI_Datatype)0x4c00043d)
#define MPI_UINT64_T          ((MPI_Datatype)0x4c00083e)

/* other C99 types */
#define MPI_C_BOOL                 ((MPI_Datatype)0x4c00013f)
#define MPI_C_FLOAT_COMPLEX        ((MPI_Datatype)0x4c000840)
#define MPI_C_COMPLEX              MPI_C_FLOAT_COMPLEX
#define MPI_C_DOUBLE_COMPLEX       ((MPI_Datatype)0x4c001041)
#define MPI_C_LONG_DOUBLE_COMPLEX  ((MPI_Datatype)0x4c002042)
/* other extension types */
#define MPIX_C_FLOAT16             ((MPI_Datatype)0x4c000246)

/* address/offset types */
#define MPI_AINT          ((MPI_Datatype)0x4c000843)
#define MPI_OFFSET        ((MPI_Datatype)0x4c000844)
#define MPI_COUNT         ((MPI_Datatype)0x4c000845)

/* MPI-3 C++ types */
#define MPI_CXX_BOOL                ((MPI_Datatype)0x4c000133)
#define MPI_CXX_FLOAT_COMPLEX       ((MPI_Datatype)0x4c000834)
#define MPI_CXX_DOUBLE_COMPLEX      ((MPI_Datatype)0x4c001035)
#define MPI_CXX_LONG_DOUBLE_COMPLEX ((MPI_Datatype)0x4c002036)

/* Communicators */
typedef int MPI_Comm;
#define MPI_COMM_WORLD ((MPI_Comm)0x44000000)
#define MPI_COMM_SELF  ((MPI_Comm)0x44000001)

/* Groups */
typedef int MPI_Group;
#define MPI_GROUP_EMPTY ((MPI_Group)0x48000000)

/* RMA and Windows */
typedef int MPI_Win;
#define MPI_WIN_NULL ((MPI_Win)0x20000000)

/* for session */
typedef int MPI_Session;
#define MPI_SESSION_NULL     ((MPI_Session)0x38000000)

/* File and IO */
/* This define lets ROMIO know that MPI_File has been defined */
#define MPI_FILE_DEFINED
/* ROMIO uses a pointer for MPI_File objects.  This must be the same definition
   as in src/mpi/romio/include/mpio.h.in  */
typedef struct ADIOI_FileD *MPI_File;
#define MPI_FILE_NULL ((MPI_File)0)

/* Collective operations */
typedef int MPI_Op;

#define MPI_MAX     (MPI_Op)(0x58000001)
#define MPI_MIN     (MPI_Op)(0x58000002)
#define MPI_SUM     (MPI_Op)(0x58000003)
#define MPI_PROD    (MPI_Op)(0x58000004)
#define MPI_LAND    (MPI_Op)(0x58000005)
#define MPI_BAND    (MPI_Op)(0x58000006)
#define MPI_LOR     (MPI_Op)(0x58000007)
#define MPI_BOR     (MPI_Op)(0x58000008)
#define MPI_LXOR    (MPI_Op)(0x58000009)
#define MPI_BXOR    (MPI_Op)(0x5800000a)
#define MPI_MINLOC  (MPI_Op)(0x5800000b)
#define MPI_MAXLOC  (MPI_Op)(0x5800000c)
#define MPI_REPLACE (MPI_Op)(0x5800000d)
#define MPI_NO_OP   (MPI_Op)(0x5800000e)
#define MPIX_EQUAL  (MPI_Op)(0x5800000f)

/* MPI errhandler objects */
typedef int MPI_Errhandler;

/* Built in (0x1 in 30-31), errhandler (0x5 in bits 26-29, allkind (0
   in 22-25), index in the low bits */
#define MPI_ERRORS_ARE_FATAL ((MPI_Errhandler)0x54000000)
#define MPI_ERRORS_RETURN    ((MPI_Errhandler)0x54000001)
/* MPIR_ERRORS_THROW_EXCEPTIONS is not part of the MPI standard, it is here to
   facilitate the c++ binding which has MPI::ERRORS_THROW_EXCEPTIONS.
   Using the MPIR prefix preserved the MPI_ names for objects defined by
   the standard. */
#define MPIR_ERRORS_THROW_EXCEPTIONS ((MPI_Errhandler)0x54000002)
#define MPI_ERRORS_ABORT     ((MPI_Errhandler)0x54000003)

/* MPI request objects */
typedef int MPI_Request;

/* MPI message objects for Mprobe and related functions */
typedef int MPI_Message;

/* Generalized requests extensions */
typedef int MPIX_Grequest_class;

/* MPI stream objects */
typedef int MPIX_Stream;

/* for info */
typedef int MPI_Info;
#define MPI_INFO_NULL         ((MPI_Info)0x1c000000)
#define MPI_INFO_ENV          ((MPI_Info)0x5c000001)

/* Permanent key values */
#define MPI_KEYVAL_INVALID 0x24000000

/* C Versions (return pointer to value),
   Fortran Versions (return integer value).
   Handled directly by the attribute value routine

   DO NOT CHANGE THESE.  The values encode:
   builtin kind (0x1 in bit 30-31)
   Keyval object (0x9 in bits 26-29)
   for communicator (0x1 in bits 22-25)

   Fortran versions of the attributes are formed by adding one to
   the C version.
 */
#define MPI_TAG_UB           0x64400001
#define MPI_HOST             0x64400003
#define MPI_IO               0x64400005
#define MPI_WTIME_IS_GLOBAL  0x64400007
#define MPI_UNIVERSE_SIZE    0x64400009
#define MPI_LASTUSEDCODE     0x6440000b
#define MPI_APPNUM           0x6440000d

/* In addition, there are 5 predefined window attributes that are
   defined for every window */
#define MPI_WIN_BASE          0x66000001
#define MPI_WIN_SIZE          0x66000003
#define MPI_WIN_DISP_UNIT     0x66000005
#define MPI_WIN_CREATE_FLAVOR 0x66000007
#define MPI_WIN_MODEL         0x66000009

#ifdef MPICH_DEFINE_ATTR_TYPE_TYPES
static const MPI_Datatype mpich_mpi_datatype_null MPICH_ATTR_TYPE_TAG_MUST_BE_NULL() = MPI_DATATYPE_NULL;
static const MPI_Datatype mpich_mpi_char               MPICH_ATTR_TYPE_TAG(char)               = MPI_CHAR;
static const MPI_Datatype mpich_mpi_signed_char        MPICH_ATTR_TYPE_TAG(signed char)        = MPI_SIGNED_CHAR;
static const MPI_Datatype mpich_mpi_unsigned_char      MPICH_ATTR_TYPE_TAG(unsigned char)      = MPI_UNSIGNED_CHAR;
/*static const MPI_Datatype mpich_mpi_byte               MPICH_ATTR_TYPE_TAG(char)               = MPI_BYTE;*/
static const MPI_Datatype mpich_mpi_wchar              MPICH_ATTR_TYPE_TAG(wchar_t)            = MPI_WCHAR;
static const MPI_Datatype mpich_mpi_short              MPICH_ATTR_TYPE_TAG(short)              = MPI_SHORT;
static const MPI_Datatype mpich_mpi_unsigned_short     MPICH_ATTR_TYPE_TAG(unsigned short)     = MPI_UNSIGNED_SHORT;
static const MPI_Datatype mpich_mpi_int                MPICH_ATTR_TYPE_TAG(int)                = MPI_INT;
static const MPI_Datatype mpich_mpi_unsigned           MPICH_ATTR_TYPE_TAG(unsigned)           = MPI_UNSIGNED;
static const MPI_Datatype mpich_mpi_long               MPICH_ATTR_TYPE_TAG(long)               = MPI_LONG;
static const MPI_Datatype mpich_mpi_unsigned_long      MPICH_ATTR_TYPE_TAG(unsigned long)      = MPI_UNSIGNED_LONG;
static const MPI_Datatype mpich_mpi_float              MPICH_ATTR_TYPE_TAG(float)              = MPI_FLOAT;
static const MPI_Datatype mpich_mpi_double             MPICH_ATTR_TYPE_TAG(double)             = MPI_DOUBLE;
#if 0x4c00100c != 0x0c000000
static const MPI_Datatype mpich_mpi_long_double        MPICH_ATTR_TYPE_TAG(long double)        = MPI_LONG_DOUBLE;
#endif
static const MPI_Datatype mpich_mpi_long_long_int      MPICH_ATTR_TYPE_TAG(long long int)      = MPI_LONG_LONG_INT;
static const MPI_Datatype mpich_mpi_unsigned_long_long MPICH_ATTR_TYPE_TAG(unsigned long long) = MPI_UNSIGNED_LONG_LONG;
struct mpich_struct_mpi_float_int       { float f; int i; };
struct mpich_struct_mpi_double_int      { double d; int i; };
struct mpich_struct_mpi_long_int        { long l; int i; };
struct mpich_struct_mpi_short_int       { short s; int i; };
struct mpich_struct_mpi_2int            { int i1; int i2; };
#if 0x8c000004 != 0x0c000000
struct mpich_struct_mpi_long_double_int { long double ld; int i; };
#endif

static const MPI_Datatype mpich_mpi_float_int       MPICH_ATTR_TYPE_TAG_LAYOUT_COMPATIBLE(struct mpich_struct_mpi_float_int)       = MPI_FLOAT_INT;
static const MPI_Datatype mpich_mpi_double_int      MPICH_ATTR_TYPE_TAG_LAYOUT_COMPATIBLE(struct mpich_struct_mpi_double_int)      = MPI_DOUBLE_INT;
static const MPI_Datatype mpich_mpi_long_int        MPICH_ATTR_TYPE_TAG_LAYOUT_COMPATIBLE(struct mpich_struct_mpi_long_int)        = MPI_LONG_INT;
static const MPI_Datatype mpich_mpi_short_int       MPICH_ATTR_TYPE_TAG_LAYOUT_COMPATIBLE(struct mpich_struct_mpi_short_int)       = MPI_SHORT_INT;

/*
 * The MPI_2INT line is commented out because currently Clang 3.3 flags
 * struct {int i1; int i2;} as different from int[2]. But actually these
 * two types are of the same layout. Clang gives a type mismatch warning
 * for a definitely correct code like the following:
 *  int in[2], out[2];
 *  MPI_Reduce(in, out, 1, MPI_2INT, MPI_MAXLOC, 0, MPI_COMM_WORLD);
 *
 * So, we disable type checking for MPI_2INT until Clang fixes this bug.
 */

/* static const MPI_Datatype mpich_mpi_2int            MPICH_ATTR_TYPE_TAG_LAYOUT_COMPATIBLE(struct mpich_struct_mpi_2int)            = MPI_2INT
 */

#if 0x8c000004 != 0x0c000000
static const MPI_Datatype mpich_mpi_long_double_int MPICH_ATTR_TYPE_TAG_LAYOUT_COMPATIBLE(struct mpich_struct_mpi_long_double_int) = MPI_LONG_DOUBLE_INT;
#endif

static const MPI_Datatype mpich_mpi_int8_t   MPICH_ATTR_TYPE_TAG_STDINT(int8_t)   = MPI_INT8_T;
static const MPI_Datatype mpich_mpi_int16_t  MPICH_ATTR_TYPE_TAG_STDINT(int16_t)  = MPI_INT16_T;
static const MPI_Datatype mpich_mpi_int32_t  MPICH_ATTR_TYPE_TAG_STDINT(int32_t)  = MPI_INT32_T;
static const MPI_Datatype mpich_mpi_int64_t  MPICH_ATTR_TYPE_TAG_STDINT(int64_t)  = MPI_INT64_T;
static const MPI_Datatype mpich_mpi_uint8_t  MPICH_ATTR_TYPE_TAG_STDINT(uint8_t)  = MPI_UINT8_T;
static const MPI_Datatype mpich_mpi_uint16_t MPICH_ATTR_TYPE_TAG_STDINT(uint16_t) = MPI_UINT16_T;
static const MPI_Datatype mpich_mpi_uint32_t MPICH_ATTR_TYPE_TAG_STDINT(uint32_t) = MPI_UINT32_T;
static const MPI_Datatype mpich_mpi_uint64_t MPICH_ATTR_TYPE_TAG_STDINT(uint64_t) = MPI_UINT64_T;

static const MPI_Datatype mpich_mpi_c_bool                MPICH_ATTR_TYPE_TAG_C99(_Bool)           = MPI_C_BOOL;
static const MPI_Datatype mpich_mpi_c_float_complex       MPICH_ATTR_TYPE_TAG_C99(float _Complex)  = MPI_C_FLOAT_COMPLEX;
static const MPI_Datatype mpich_mpi_c_double_complex      MPICH_ATTR_TYPE_TAG_C99(double _Complex) = MPI_C_DOUBLE_COMPLEX;
#if 0x4c002042 != 0x0c000000
static const MPI_Datatype mpich_mpi_c_long_double_complex MPICH_ATTR_TYPE_TAG_C99(long double _Complex) = MPI_C_LONG_DOUBLE_COMPLEX;
#endif

static const MPI_Datatype mpich_mpi_aint   MPICH_ATTR_TYPE_TAG(MPI_Aint)   = MPI_AINT;
static const MPI_Datatype mpich_mpi_offset MPICH_ATTR_TYPE_TAG(MPI_Offset) = MPI_OFFSET;
#endif

/* Definitions that are determined by configure. */
typedef long MPI_Aint;
typedef int MPI_Fint;
typedef long long MPI_Count;

/* Let ROMIO know that MPI_Offset is already defined */
#define HAVE_MPI_OFFSET
/* MPI_OFFSET_TYPEDEF is set in configure and is
      typedef $MPI_OFFSET MPI_Offset;
   where $MPI_OFFSET is the correct C type */
typedef long long MPI_Offset;

/* The order of these elements must match that in mpif.h, mpi_f08_types.f90,
   and mpi_c_interface_types.f90 */
typedef struct MPI_Status {
    int count_lo;
    int count_hi_and_cancelled;
    int MPI_SOURCE;
    int MPI_TAG;
    int MPI_ERROR;
} MPI_Status;

/* See 4.12.5 for MPI_F_STATUS(ES)_IGNORE */
extern MPIU_DLL_SPEC MPI_Fint * MPI_F_STATUS_IGNORE MPICH_API_PUBLIC;
extern MPIU_DLL_SPEC MPI_Fint * MPI_F_STATUSES_IGNORE MPICH_API_PUBLIC;
/* The annotation MPIU_DLL_SPEC to the extern statements is used
   as a hook for systems that require C extensions to correctly construct
   DLLs, and is defined as an empty string otherwise
 */

/* C type for MPI_STATUS in F08.
   The field order should match that in mpi_f08_types.f90, and mpi_c_interface_types.f90.
 */
typedef struct {
    MPI_Fint count_lo;
    MPI_Fint count_hi_and_cancelled;
    MPI_Fint MPI_SOURCE;
    MPI_Fint MPI_TAG;
    MPI_Fint MPI_ERROR;
} MPI_F08_status;

/* MPI 4 added following constants to allow access F90 STATUS as an array of MPI_Fint */
#define MPI_F_STATUS_SIZE 5
#define MPI_F_SOURCE 2
#define MPI_F_TAG 3
#define MPI_F_ERROR 4

/* Provided in libmpifort.so */
extern MPI_F08_status *MPI_F08_STATUS_IGNORE;
extern MPI_F08_status *MPI_F08_STATUSES_IGNORE;

/* FIXME: The following two definition are not defined by MPI and must not be
   included in the mpi.h file, as the MPI namespace is reserved to the MPI
   standard */
#define MPI_AINT_FMT_DEC_SPEC "%ld"
#define MPI_AINT_FMT_HEX_SPEC "%lx"

/* These are only guesses; make sure you change them in mpif.h as well */
#define MPI_MAX_PROCESSOR_NAME 128
#define MPI_MAX_LIBRARY_VERSION_STRING 8192
#define MPI_MAX_ERROR_STRING   512
#define MPI_MAX_PORT_NAME      256
#define MPI_MAX_OBJECT_NAME    128
#define MPI_MAX_STRINGTAG_LEN  256
#define MPI_MAX_PSET_NAME_LEN  256
#define MPI_MAX_INFO_KEY       255
#define MPI_MAX_INFO_VAL      1024

/* MPI-3 window flavors */
typedef enum MPIR_Win_flavor {
    MPI_WIN_FLAVOR_CREATE      = 1,
    MPI_WIN_FLAVOR_ALLOCATE    = 2,
    MPI_WIN_FLAVOR_DYNAMIC     = 3,
    MPI_WIN_FLAVOR_SHARED      = 4
} MPIR_Win_flavor_t;

/* MPI-3 window consistency models */
typedef enum MPIR_Win_model {
    MPI_WIN_SEPARATE   = 1,
    MPI_WIN_UNIFIED    = 2
} MPIR_Win_model_t;

/* Upper bound on the overhead in bsend for each message buffer */
#define MPI_BSEND_OVERHEAD 96

/* Topology types */
typedef enum MPIR_Topo_type { MPI_GRAPH=1, MPI_CART=2, MPI_DIST_GRAPH=3 } MPIR_Topo_type;

extern int * const MPI_UNWEIGHTED MPICH_API_PUBLIC;
extern int * const MPI_WEIGHTS_EMPTY MPICH_API_PUBLIC;

#define MPI_PROC_NULL   (-1)
#define MPI_ANY_SOURCE 	(-2)
#define MPI_ROOT        (-3)
#define MPI_ANY_TAG     (-1)

#define MPI_BOTTOM      (void *)0
#define MPI_IN_PLACE  (void *) -1
#define MPI_BUFFER_AUTOMATIC (void *) -2

#define MPI_UNDEFINED      (-32766)

#define MPI_STATUS_IGNORE (MPI_Status *)1
#define MPI_STATUSES_IGNORE (MPI_Status *)1
#define MPI_ERRCODES_IGNORE (int *)0

/* The MPI standard requires that the ARGV_NULL values be the same as
   NULL (see 5.3.2) */
#define MPI_ARGV_NULL (char **)0
#define MPI_ARGVS_NULL (char ***)0

/* Results of the compare operations. */
#define MPI_IDENT     0
#define MPI_CONGRUENT 1
#define MPI_SIMILAR   2
#define MPI_UNEQUAL   3

/* typeclasses */
#define MPI_TYPECLASS_REAL 1
#define MPI_TYPECLASS_INTEGER 2
#define MPI_TYPECLASS_COMPLEX 3

#define MPI_LOCK_EXCLUSIVE  234
#define MPI_LOCK_SHARED     235

/* for the datatype decoders */
enum MPIR_Combiner_enum {
    MPI_COMBINER_NAMED            = 1,
    MPI_COMBINER_DUP              = 2,
    MPI_COMBINER_CONTIGUOUS       = 3,
    MPI_COMBINER_VECTOR           = 4,
    MPI_COMBINER_HVECTOR_INTEGER  = 5,
    MPI_COMBINER_HVECTOR          = 6,
    MPI_COMBINER_INDEXED          = 7,
    MPI_COMBINER_HINDEXED_INTEGER = 8,
    MPI_COMBINER_HINDEXED         = 9,
    MPI_COMBINER_INDEXED_BLOCK    = 10,
    MPI_COMBINER_STRUCT_INTEGER   = 11,
    MPI_COMBINER_STRUCT           = 12,
    MPI_COMBINER_SUBARRAY         = 13,
    MPI_COMBINER_DARRAY           = 14,
    MPI_COMBINER_F90_REAL         = 15,
    MPI_COMBINER_F90_COMPLEX      = 16,
    MPI_COMBINER_F90_INTEGER      = 17,
    MPI_COMBINER_RESIZED          = 18,
    MPI_COMBINER_HINDEXED_BLOCK   = 19,
    MPI_COMBINER_VALUE_INDEX      = 20
};

/* for subarray and darray constructors */
#define MPI_ORDER_C              56
#define MPI_ORDER_FORTRAN        57
#define MPI_DISTRIBUTE_BLOCK    121
#define MPI_DISTRIBUTE_CYCLIC   122
#define MPI_DISTRIBUTE_NONE     123
#define MPI_DISTRIBUTE_DFLT_DARG -49767

/* asserts for one-sided communication */
#define MPI_MODE_NOCHECK      1024
#define MPI_MODE_NOSTORE      2048
#define MPI_MODE_NOPUT        4096
#define MPI_MODE_NOPRECEDE    8192
#define MPI_MODE_NOSUCCEED   16384

/* predefined types for MPI_Comm_split_type */
#define MPI_COMM_TYPE_SHARED    1
#define MPI_COMM_TYPE_HW_GUIDED 2
#define MPI_COMM_TYPE_HW_UNGUIDED 3
#define MPI_COMM_TYPE_RESOURCE_GUIDED 4

/* MPICH-specific types */
#define MPIX_COMM_TYPE_NEIGHBORHOOD 5

/* For supported thread levels */
#define MPI_THREAD_SINGLE 0
#define MPI_THREAD_FUNNELED 1
#define MPI_THREAD_SERIALIZED 2
#define MPI_THREAD_MULTIPLE 3

/* MPI's error classes */
#define MPI_SUCCESS          0      /* Successful return code */
/* Communication argument parameters */
#define MPI_ERR_BUFFER       1      /* Invalid buffer pointer */
#define MPI_ERR_COUNT        2      /* Invalid count argument */
#define MPI_ERR_TYPE         3      /* Invalid datatype argument */
#define MPI_ERR_TAG          4      /* Invalid tag argument */
#define MPI_ERR_COMM         5      /* Invalid communicator */
#define MPI_ERR_RANK         6      /* Invalid rank */
#define MPI_ERR_ROOT         7      /* Invalid root */
#define MPI_ERR_TRUNCATE    14      /* Message truncated on receive */

/* MPI Objects (other than COMM) */
#define MPI_ERR_GROUP        8      /* Invalid group */
#define MPI_ERR_OP           9      /* Invalid operation */
#define MPI_ERR_REQUEST     19      /* Invalid mpi_request handle */

/* Special topology argument parameters */
#define MPI_ERR_TOPOLOGY    10      /* Invalid topology */
#define MPI_ERR_DIMS        11      /* Invalid dimension argument */

/* All other arguments.  This is a class with many kinds */
#define MPI_ERR_ARG         12      /* Invalid argument */

/* Other errors that are not simply an invalid argument */
#define MPI_ERR_OTHER       15      /* Other error; use Error_string */

#define MPI_ERR_UNKNOWN     13      /* Unknown error */
#define MPI_ERR_INTERN      16      /* Internal error code    */

/* Multiple completion has three special error classes */
#define MPI_ERR_IN_STATUS           17      /* Look in status for error value */
#define MPI_ERR_PENDING             18      /* Pending request */

/* New MPI-2 Error classes */
#define MPI_ERR_ACCESS      20      /* */
#define MPI_ERR_AMODE       21      /* */
#define MPI_ERR_BAD_FILE    22      /* */
#define MPI_ERR_CONVERSION  23      /* */
#define MPI_ERR_DUP_DATAREP 24      /* */
#define MPI_ERR_FILE_EXISTS 25      /* */
#define MPI_ERR_FILE_IN_USE 26      /* */
#define MPI_ERR_FILE        27      /* */
#define MPI_ERR_IO          32      /* */
#define MPI_ERR_NO_SPACE    36      /* */
#define MPI_ERR_NO_SUCH_FILE 37     /* */
#define MPI_ERR_READ_ONLY   40      /* */
#define MPI_ERR_UNSUPPORTED_DATAREP   43  /* */

/* MPI_ERR_INFO is NOT defined in the MPI-2 standard.  I believe that
   this is an oversight */
#define MPI_ERR_INFO        28      /* */
#define MPI_ERR_INFO_KEY    29      /* */
#define MPI_ERR_INFO_VALUE  30      /* */
#define MPI_ERR_INFO_NOKEY  31      /* */

#define MPI_ERR_NAME        33      /* */
#define MPI_ERR_NO_MEM      34      /* Alloc_mem could not allocate memory */
#define MPI_ERR_NOT_SAME    35      /* */
#define MPI_ERR_PORT        38      /* */
#define MPI_ERR_QUOTA       39      /* */
#define MPI_ERR_SERVICE     41      /* */
#define MPI_ERR_SPAWN       42      /* */
#define MPI_ERR_UNSUPPORTED_OPERATION 44 /* */
#define MPI_ERR_WIN         45      /* */

#define MPI_ERR_BASE        46      /* */
#define MPI_ERR_LOCKTYPE    47      /* */
#define MPI_ERR_KEYVAL      48      /* Erroneous attribute key */
#define MPI_ERR_RMA_CONFLICT 49     /* */
#define MPI_ERR_RMA_SYNC    50      /* */
#define MPI_ERR_SIZE        51      /* */
#define MPI_ERR_DISP        52      /* */
#define MPI_ERR_ASSERT      53      /* */

#define MPI_ERR_RMA_RANGE  55       /* */
#define MPI_ERR_RMA_ATTACH 56       /* */
#define MPI_ERR_RMA_SHARED 57       /* */
#define MPI_ERR_RMA_FLAVOR 58       /* */

/* Return codes for functions in the MPI Tool Information Interface */
#define MPI_T_ERR_MEMORY            59  /* Out of memory */
#define MPI_T_ERR_NOT_INITIALIZED   60  /* Interface not initialized */
#define MPI_T_ERR_CANNOT_INIT       61  /* Interface not in the state to
                                           be initialized */
#define MPI_T_ERR_INVALID_INDEX     62  /* The index is invalid or
                                           has been deleted  */
#define MPI_T_ERR_INVALID_ITEM      63  /* Deprecated.  If a queried item index is out of range,
                                         * MPI-4 will return MPI_T_ERR_INVALID_INDEX instead. */
#define MPI_T_ERR_INVALID_HANDLE    64  /* The handle is invalid */
#define MPI_T_ERR_OUT_OF_HANDLES    65  /* No more handles available */
#define MPI_T_ERR_OUT_OF_SESSIONS   66  /* No more sessions available */
#define MPI_T_ERR_INVALID_SESSION   67  /* Session argument is not valid */
#define MPI_T_ERR_CVAR_SET_NOT_NOW  68  /* Cvar can't be set at this moment */
#define MPI_T_ERR_CVAR_SET_NEVER    69  /* Cvar can't be set until
                                           end of execution */
#define MPI_T_ERR_PVAR_NO_STARTSTOP 70  /* Pvar can't be started or stopped */
#define MPI_T_ERR_PVAR_NO_WRITE     71  /* Pvar can't be written or reset */
#define MPI_T_ERR_PVAR_NO_ATOMIC    72  /* Pvar can't be R/W atomically */
#define MPI_T_ERR_INVALID_NAME      73  /* Name doesn't match */
#define MPI_T_ERR_INVALID           74  /* Generic error code for MPI_T added in MPI-3.1 */

#define MPI_ERR_SESSION            75  /* Invalid session handle */
#define MPI_ERR_PROC_ABORTED       76  /* Trying to communicate with aborted processes */
#define MPI_ERR_VALUE_TOO_LARGE    77  /* Value is too large to store */

#define MPI_T_ERR_NOT_SUPPORTED    78  /* Requested functionality not supported */
#define MPI_T_ERR_NOT_ACCESSIBLE   79  /* Requested functionality not accessible */

#define MPI_ERR_ERRHANDLER         80  /* Invalid errhandler handle */

#define MPI_ERR_LASTCODE    0x3fffffff  /* Last valid error code for a
					   predefined error class */
#define MPICH_ERR_LAST_CLASS 80     /* It is also helpful to know the
				       last valid class */

#define MPICH_ERR_FIRST_MPIX 100 /* Define a gap here because sock is
                                  * already using some of the values in this
                                  * range. All MPIX error codes will be
                                  * above this value to be ABI complaint. */

#define MPIX_ERR_PROC_FAILED          MPICH_ERR_FIRST_MPIX+1 /* Process failure */
#define MPIX_ERR_PROC_FAILED_PENDING  MPICH_ERR_FIRST_MPIX+2 /* A failure has caused this request
                                                              * to be pending */
#define MPIX_ERR_REVOKED              MPICH_ERR_FIRST_MPIX+3 /* The communication object has been revoked */
#define MPIX_ERR_EAGAIN               MPICH_ERR_FIRST_MPIX+4 /* Operation could not be issued */
#define MPIX_ERR_NOREQ                MPICH_ERR_FIRST_MPIX+5 /* Cannot allocate request */
#define MPIX_ERR_STREAM               MPICH_ERR_FIRST_MPIX+6 /* Invalid stream */
#define MPIX_ERR_TIMEOUT              MPICH_ERR_FIRST_MPIX+7 /* Operation timed out */

#define MPICH_ERR_LAST_MPIX           MPICH_ERR_FIRST_MPIX+7

/* End of MPI's error classes */

/* GPU extensions */
#define MPIX_GPU_SUPPORT_CUDA  (0)
#define MPIX_GPU_SUPPORT_ZE    (1)
#define MPIX_GPU_SUPPORT_HIP   (2)

/* feature advertisement */
#define MPIIMPL_ADVERTISES_FEATURES 1
#define MPIIMPL_HAVE_MPI_INFO 1
#define MPIIMPL_HAVE_MPI_COMBINER_DARRAY 1
#define MPIIMPL_HAVE_MPI_TYPE_CREATE_DARRAY 1
#define MPIIMPL_HAVE_MPI_COMBINER_SUBARRAY 1
#define MPIIMPL_HAVE_MPI_TYPE_CREATE_DARRAY 1
#define MPIIMPL_HAVE_MPI_COMBINER_DUP 1
#define MPIIMPL_HAVE_MPI_GREQUEST 1
#define MPIIMPL_HAVE_STATUS_SET_BYTES 1
#define MPIIMPL_HAVE_STATUS_SET_INFO 1

/* C callback functions */
typedef void (MPI_Handler_function) ( MPI_Comm *, int *, ... );
typedef int (MPI_Comm_copy_attr_function)(MPI_Comm, int, void *, void *,
					  void *, int *);
typedef int (MPI_Comm_delete_attr_function)(MPI_Comm, int, void *, void *);
typedef int (MPI_Type_copy_attr_function)(MPI_Datatype, int, void *, void *,
					  void *, int *);
typedef int (MPI_Type_delete_attr_function)(MPI_Datatype, int, void *, void *);
typedef int (MPI_Win_copy_attr_function)(MPI_Win, int, void *, void *, void *,
					 int *);
typedef int (MPI_Win_delete_attr_function)(MPI_Win, int, void *, void *);
/* added in MPI-2.2 */
typedef void (MPI_Comm_errhandler_function)(MPI_Comm *, int *, ...);
typedef void (MPI_File_errhandler_function)(MPI_File *, int *, ...);
typedef void (MPI_Win_errhandler_function)(MPI_Win *, int *, ...);
typedef void (MPI_Session_errhandler_function)(MPI_Session *, int *, ...);
/* names that were added in MPI-2.0 and deprecated in MPI-2.2 */
typedef MPI_Comm_errhandler_function MPI_Comm_errhandler_fn;
typedef MPI_File_errhandler_function MPI_File_errhandler_fn;
typedef MPI_Win_errhandler_function MPI_Win_errhandler_fn;
typedef MPI_Session_errhandler_function MPI_Session_errhandler_fn;

/* MPI Attribute copy and delete functions */
typedef int (MPI_Copy_function) ( MPI_Comm, int, void *, void *, void *, int * );
typedef int (MPI_Delete_function) ( MPI_Comm, int, void *, void * );

/* User combination function */
typedef void (MPI_User_function) ( void *, void *, int *, MPI_Datatype * );
typedef void (MPI_User_function_c) ( void *, void *, MPI_Count *, MPI_Datatype * );

/* Typedefs for generalized requests */
typedef int (MPI_Grequest_cancel_function)(void *, int);
typedef int (MPI_Grequest_free_function)(void *);
typedef int (MPI_Grequest_query_function)(void *, MPI_Status *);
typedef int (MPIX_Grequest_poll_function)(void *, MPI_Status *);
typedef int (MPIX_Grequest_wait_function)(int, void **, double, MPI_Status *);

/* Function type defs */
typedef int (MPI_Datarep_conversion_function)(void *, MPI_Datatype, int,
             void *, MPI_Offset, void *);
typedef int (MPI_Datarep_extent_function)(MPI_Datatype datatype, MPI_Aint *,
                      void *);
typedef int (MPI_Datarep_conversion_function_c)(void *, MPI_Datatype, MPI_Count,
             void *, MPI_Offset, void *);

/* Make the C names for the dup function mixed case.
   This is required for systems that use all uppercase names for Fortran
   externals.  */
/* MPI 1 names */
#define MPI_NULL_COPY_FN   ((MPI_Copy_function *)0)
#define MPI_NULL_DELETE_FN ((MPI_Delete_function *)0)
#define MPI_DUP_FN         MPIR_Dup_fn
/* MPI 2 names */
#define MPI_COMM_NULL_COPY_FN ((MPI_Comm_copy_attr_function*)0)
#define MPI_COMM_NULL_DELETE_FN ((MPI_Comm_delete_attr_function*)0)
#define MPI_COMM_DUP_FN  ((MPI_Comm_copy_attr_function *)MPI_DUP_FN)
#define MPI_WIN_NULL_COPY_FN ((MPI_Win_copy_attr_function*)0)
#define MPI_WIN_NULL_DELETE_FN ((MPI_Win_delete_attr_function*)0)
#define MPI_WIN_DUP_FN   ((MPI_Win_copy_attr_function*)MPI_DUP_FN)
#define MPI_TYPE_NULL_COPY_FN ((MPI_Type_copy_attr_function*)0)
#define MPI_TYPE_NULL_DELETE_FN ((MPI_Type_delete_attr_function*)0)
#define MPI_TYPE_DUP_FN ((MPI_Type_copy_attr_function*)MPI_DUP_FN)

#define MPI_CONVERSION_FN_NULL ((MPI_Datarep_conversion_function *)0)
#define MPI_CONVERSION_FN_NULL_C ((MPI_Datarep_conversion_function_c *)0)

/* types for the MPI_T_ interface */
struct MPIR_T_enum_s;
struct MPIR_T_cvar_handle_s;
struct MPIR_T_pvar_handle_s;
struct MPIR_T_pvar_session_s;
struct MPIR_T_event_registration_s;
struct MPIR_T_event_instance_s;

typedef struct MPIR_T_enum_s * MPI_T_enum;
typedef struct MPIR_T_cvar_handle_s * MPI_T_cvar_handle;
typedef struct MPIR_T_pvar_handle_s * MPI_T_pvar_handle;
typedef struct MPIR_T_pvar_session_s * MPI_T_pvar_session;
typedef struct MPIR_T_event_registration_s * MPI_T_event_registration;
typedef struct MPIR_T_event_instance_s * MPI_T_event_instance;

/* extra const at front would be safer, but is incompatible with MPI_T_ prototypes */
extern struct MPIR_T_pvar_handle_s * const MPI_T_PVAR_ALL_HANDLES MPICH_API_PUBLIC;

#define MPI_T_ENUM_NULL         ((MPI_T_enum)NULL)
#define MPI_T_CVAR_HANDLE_NULL  ((MPI_T_cvar_handle)NULL)
#define MPI_T_PVAR_HANDLE_NULL  ((MPI_T_pvar_handle)NULL)
#define MPI_T_PVAR_SESSION_NULL ((MPI_T_pvar_session)NULL)

/* the MPI_T_ interface requires that these VERBOSITY constants occur in this
 * relative order with increasing values */
typedef enum MPIR_T_verbosity_t {
    /* don't name-shift this if/when MPI_T_ is accepted, this is an MPICH-only
     * extension */
    MPI_T_VERBOSITY_INVALID = 0,

    /* arbitrarily shift values to aid debugging and reduce accidental errors */
    MPI_T_VERBOSITY_USER_BASIC = 221,
    MPI_T_VERBOSITY_USER_DETAIL,
    MPI_T_VERBOSITY_USER_ALL,

    MPI_T_VERBOSITY_TUNER_BASIC,
    MPI_T_VERBOSITY_TUNER_DETAIL,
    MPI_T_VERBOSITY_TUNER_ALL,

    MPI_T_VERBOSITY_MPIDEV_BASIC,
    MPI_T_VERBOSITY_MPIDEV_DETAIL,
    MPI_T_VERBOSITY_MPIDEV_ALL
} MPIR_T_verbosity_t;

typedef enum MPIR_T_bind_t {
    /* don't name-shift this if/when MPI_T_ is accepted, this is an MPICH-only
     * extension */
    MPI_T_BIND_INVALID = 0,

    /* arbitrarily shift values to aid debugging and reduce accidental errors */
    MPI_T_BIND_NO_OBJECT = 9700,
    MPI_T_BIND_MPI_COMM,
    MPI_T_BIND_MPI_DATATYPE,
    MPI_T_BIND_MPI_ERRHANDLER,
    MPI_T_BIND_MPI_FILE,
    MPI_T_BIND_MPI_GROUP,
    MPI_T_BIND_MPI_OP,
    MPI_T_BIND_MPI_REQUEST,
    MPI_T_BIND_MPI_WIN,
    MPI_T_BIND_MPI_MESSAGE,
    MPI_T_BIND_MPI_INFO,
    MPI_T_BIND_MPI_SESSION
} MPIR_T_bind_t;

typedef enum MPIR_T_scope_t {
    /* don't name-shift this if/when MPI_T_ is accepted, this is an MPICH-only
     * extension */
    MPI_T_SCOPE_INVALID = 0,

    /* arbitrarily shift values to aid debugging and reduce accidental errors */
    MPI_T_SCOPE_CONSTANT = 60438,
    MPI_T_SCOPE_READONLY,
    MPI_T_SCOPE_LOCAL,
    MPI_T_SCOPE_GROUP,
    MPI_T_SCOPE_GROUP_EQ,
    MPI_T_SCOPE_ALL,
    MPI_T_SCOPE_ALL_EQ
} MPIR_T_scope_t;

typedef enum MPIR_T_pvar_class_t {
    /* don't name-shift this if/when MPI_T_ is accepted, this is an MPICH-only
     * extension */
    MPI_T_PVAR_CLASS_INVALID = 0,

    /* arbitrarily shift values to aid debugging and reduce accidental errors */
    MPIR_T_PVAR_CLASS_FIRST = 240,
    MPI_T_PVAR_CLASS_STATE = MPIR_T_PVAR_CLASS_FIRST,
    MPI_T_PVAR_CLASS_LEVEL,
    MPI_T_PVAR_CLASS_SIZE,
    MPI_T_PVAR_CLASS_PERCENTAGE,
    MPI_T_PVAR_CLASS_HIGHWATERMARK,
    MPI_T_PVAR_CLASS_LOWWATERMARK,
    MPI_T_PVAR_CLASS_COUNTER,
    MPI_T_PVAR_CLASS_AGGREGATE,
    MPI_T_PVAR_CLASS_TIMER,
    MPI_T_PVAR_CLASS_GENERIC,
    MPIR_T_PVAR_CLASS_LAST,
    MPIR_T_PVAR_CLASS_NUMBER = MPIR_T_PVAR_CLASS_LAST - MPIR_T_PVAR_CLASS_FIRST
} MPIR_T_pvar_class_t;

typedef enum MPI_T_cb_safety {
    MPI_T_CB_REQUIRE_NONE = 0,
    MPI_T_CB_REQUIRE_MPI_RESTRICTED,
    MPI_T_CB_REQUIRE_THREAD_SAFE,
    MPI_T_CB_REQUIRE_ASYNC_SIGNAL_SAFE
} MPI_T_cb_safety;

typedef enum MPI_T_source_order {
    MPI_T_SOURCE_ORDERED = 0,
    MPI_T_SOURCE_UNORDERED
} MPI_T_source_order;

typedef void (MPI_T_event_cb_function)(MPI_T_event_instance event_instance, MPI_T_event_registration event_registration, MPI_T_cb_safety cb_safety, void *user_data);
typedef void (MPI_T_event_free_cb_function)(MPI_T_event_registration event_registration, MPI_T_cb_safety cb_safety, void *user_data);
typedef void (MPI_T_event_dropped_cb_function)(MPI_Count count, MPI_T_event_registration event_registration, int source_index, MPI_T_cb_safety cb_safety, void *user_data);

typedef struct {
    void **storage_stack;
} QMPI_Context;

#define QMPI_MAX_TOOL_NAME_LENGTH 256

int QMPI_Register_tool_name(const char *tool_name,
                            void (*init_function_ptr) (int tool_id)) MPICH_API_PUBLIC;
int QMPI_Register_tool_storage(int tool_id, void *tool_storage) MPICH_API_PUBLIC;
int QMPI_Register_function(int calling_tool_id, int function_enum,
                           void (*function_ptr) (void)) MPICH_API_PUBLIC;
int QMPI_Get_function(int calling_tool_id, int function_enum,
                      void (**function_ptr) (void), int *next_tool_id) MPICH_API_PUBLIC;
int QMPI_Get_tool_storage(QMPI_Context context, int tool_id, void **storage) MPICH_API_PUBLIC;
int QMPI_Get_calling_address(QMPI_Context context, void **address) MPICH_API_PUBLIC;

/*
   For systems that may need to add additional definitions to support
   different declaration styles and options (e.g., different calling
   conventions or DLL import/export controls).
*/
/* --Insert Additional Definitions Here-- */

/* same as struct iovec, provided to avoid header dependency */
typedef struct MPIX_Iov {
    void *iov_base;
    MPI_Aint iov_len;
} MPIX_Iov;

/*
 * Normally, we provide prototypes for all MPI routines.  In a few weird
 * cases, we need to suppress the prototypes.
 */
#ifndef MPICH_SUPPRESS_PROTOTYPES
/* We require that the C compiler support prototypes */
#include <mpi_proto.h>
#endif /* MPICH_SUPPRESS_PROTOTYPES */

#include "mpio.h"

#if defined(__cplusplus)
}
/* Add the C++ bindings */
/*
   If MPICH_SKIP_MPICXX is defined, the mpicxx.h file will *not* be included.
   This is necessary, for example, when building the C++ interfaces.  It
   can also be used when you want to use a C++ compiler to compile C code,
   and do not want to load the C++ bindings.  These definitions can
   be made by the C++ compilation script
 */
#if !defined(MPICH_SKIP_MPICXX)
/* mpicxx.h contains the MPI C++ binding.  In the mpi.h.in file, this
   include is in an autoconf variable in case the compiler is a C++
   compiler but MPI was built without the C++ bindings */
#include "mpicxx.h"
#endif
#endif

#endif
