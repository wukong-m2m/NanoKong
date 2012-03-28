#ifndef _NVMFEATURES_H_
#define _NVMFEATURES_H_

// checking flags...
#ifdef NVM_USE_SWITCH
# ifndef NVM_USE_LOOKUPSWITCH
#  define NVM_USE_LOOKUPSWITCH
# endif
# ifndef NVM_USE_TABLESWITCH
#  define NVM_USE_TABLESWITCH
# endif
#endif

// checking int size flags
#ifdef NVM_USE_16BIT_WORD
# ifdef NVM_USE_32BIT_WORD
#  error "you must define either NVM_USE_16BIT_WORD or NVM_USE_32BIT_WORD, not both!"
# endif
#else 
# ifndef NVM_USE_32BIT_WORD
#  define NVM_USE_16BIT_WORD
//#  define NVM_USE_32BIT_WORD
# endif
#endif

// checking float flags
#ifdef NVM_USE_FLOAT
# ifndef NVM_USE_32BIT_WORD
#  error "NVM_USE_FLOAT is only allowed with NVM_USE_32BIT_WORD!"
# endif
#endif


#define NVMFILE_VERSION    2
#define NVMFILE_MAGIC      0xBE000000L


#define NVM_FEATURE_LOOKUPSWITCH (1L<<0)
#define NVM_FEATURE_TABLESWITCH  (1L<<1)
#define NVM_FEATURE_32BIT        (1L<<2)
#define NVM_FEATURE_FLOAT        (1L<<3)
#define NVM_FEATURE_ARRAY        (1L<<4)
#define NVM_FEATURE_INHERITANCE  (1L<<5)
#define NVM_FEATURE_EXTSTACK     (1L<<6)

#ifndef NVM_USE_LOOKUPSWITCH
# undef NVM_FEATURE_LOOKUPSWITCH
# define NVM_FEATURE_LOOKUPSWITCH 0
#endif

#ifndef NVM_USE_TABLESWITCH
# undef NVM_FEATURE_TABLESWITCH
# define NVM_FEATURE_TABLESWITCH 0
#endif

#ifndef NVM_USE_32BIT_WORD
# undef NVM_FEATURE_32BIT
# define NVM_FEATURE_32BIT 0
#endif

#ifndef NVM_USE_FLOAT
# undef NVM_FEATURE_FLOAT
# define NVM_FEATURE_FLOAT 0
#endif

#ifndef NVM_USE_ARRAY
# undef NVM_FEATURE_ARRAY
# define NVM_FEATURE_ARRAY 0
#endif

#ifndef NVM_USE_INHERITANCE
# undef NVM_FEATURE_INHERITANCE
# define NVM_FEATURE_INHERITANCE 0
#endif

#ifndef NVM_USE_EXTSTACKOPS
# undef NVM_FEATURE_EXTSTACK
# define NVM_FEATURE_EXTSTACK 0
#endif


#define NVM_MAGIC_FEATURE (NVMFILE_MAGIC\
                           |NVM_FEATURE_LOOKUPSWITCH\
                           |NVM_FEATURE_TABLESWITCH\
                           |NVM_FEATURE_32BIT\
                           |NVM_FEATURE_FLOAT\
                           |NVM_FEATURE_ARRAY\
                           |NVM_FEATURE_INHERITANCE)


#endif // _NVMFEATURES_H_
