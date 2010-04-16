/***************************************************************************
      kdbprivate.h  -  Private classes definition

                           -------------------
    begin                : Mon Apr 12 2004
    copyright            : (C) 2004 by Avi Alkalay
    email                : avi@unix.sh
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the BSD License (revised).                      *
 *                                                                         *
 ***************************************************************************/

#ifndef KDB_H
#error dont include kdbprivate.h directly, use kdbbackend.h instead
#else

#ifndef KDBPRIVATE_H
#define KDBPRIVATE_H

#include <limits.h>
#include <kdbloader.h>

#ifndef KDB_DB_SYSTEM
/**Below this directory the system configuration (system/) will be searched.*/
#define KDB_DB_SYSTEM            "@KDB_DB_SYSTEM@"
#endif

#ifndef KDB_DB_USER
/**This directory will be concatenated with a prefix which will be searched
 * at runtime inside kdbbGetFullFilename().
 *@see kdbbGetFullFilename
 */
#define KDB_DB_USER              ".kdb"
#endif

#ifndef KDB_DB_HOME
/**This directory will be used as fallback when no other method of
 * kdbbGetFullFilename() works.
 *@see kdbbGetFullFilename
 */
#define KDB_DB_HOME              "/home"
#endif

#ifndef KDB_KEY_MOUNTPOINTS
/**Backend information.
 *
 * This key directory tells you where each backend is mounted
 * to which mountpoint. */
#define KDB_KEY_MOUNTPOINTS      "system/elektra/mountpoints"
#endif

#define KDB_KEY_MOUNTPOINTS_LEN  (sizeof (KDB_KEY_MOUNTPOINTS))

#ifndef KDB_KEY_USERS
/**Users information.
 *
 * This key directory tells you the users existing on the system. */
#define KDB_KEY_USERS            "system/users"
#endif

#define KDB_KEY_USERS_LEN        (sizeof (KDB_KEY_USERS))

#ifndef MAX_UCHAR
#define MAX_UCHAR (UCHAR_MAX+1)
#endif

#ifndef KEYSET_SIZE
#define KEYSET_SIZE 16
#endif

#ifndef APPROXIMATE_NR_OF_BACKENDS
#define APPROXIMATE_NR_OF_BACKENDS 16
#endif

#ifndef ESCAPE_CHAR
#define ESCAPE_CHAR '\\'
#endif

/**BUFFER_SIZE can be used as value for any I/O buffer
 * on files.
 *
 * It may be used for optimization on various
 * systems.*/
#ifndef BUFFER_SIZE
#define BUFFER_SIZE 256
#endif

#ifdef UT_NAMESIZE
#define USER_NAME_SIZE UT_NAMESIZE
#else
#define USER_NAME_SIZE 100
#endif


#ifndef DEFFILEMODE
#define DEFFILEMODE 0666
#endif


#define UTF8_TO   1
#define UTF8_FROM 0


typedef struct _Trie	Trie;
typedef struct _Split	Split;
typedef struct _KDBCap	KDBCap;

/* These define the type for pointers to all the kdb functions */
typedef KDB*     (*kdbOpenPtr)();
typedef int      (*kdbClosePtr)(KDB *);

typedef ssize_t  (*kdbGetPtr)(KDB *handle, KeySet *returned, const Key *parentKey);
typedef ssize_t  (*kdbSetPtr)(KDB *handle, KeySet *returned, const Key *parentKey);

typedef KDB* (*OpenMapper)(const char *,const char *,KeySet *);
typedef int (*CloseMapper)(KDB *);



/*****************
 * Key Flags
 *****************/

/**
 * Key Flags
 *
 * Flags are used to communicate between backends and the library.
 * The flags are for internal reasons only, you should not need to
 * modify them.
 *
 * Backend <-> Library
 *
 * There are three possible ways to use the flag,
 * to communicate:
 * - from backend to library (referred as in)
 * - from library to backend (referred as out)
 * - two-way communication (referred as in/out)
 *
 * @ingroup backend
 */
typedef enum
{
	KEY_FLAG_SYNC=1,	/*!< Key need sync. (in/out)
		If value, comment or metadata
		are changed this flag will be set, so that the backend will sync
		the key to database.*/
	KEY_FLAG_REMOVE=1<<1,	/*!< Key will be removed. (out)
		The key is marked to be removed.*/
	KEY_FLAG_STAT=1<<2	/*!< Only stat the key. (in)
		don't get value and comment, if the backend supports it. */
} keyflag_t;



/*****************
 * KeySet Flags
 *****************/

/*
 * KeySet Flags
 *
 * @ingroup ks
 */
typedef enum
{
	KS_FLAG_DIRTY=1<<1	/*!< KeySet is dirty.
		* KeySet will be sorted before next kdbSet() and ksLookupByName().*/
} ksflag_t;


/**
 * The private Key struct.
 *
 * Its internal private attributes should not be accessed directly by regular
 * programs. Use the @ref key "Key access methods" instead.
 * Only a backend writer needs to have access to the private attributes of the
 * Key object which is defined as:
 * @code
typedef struct _Key Key;
 * @endcode
 *
 * @ingroup backend
 */
struct _Key {
	/**
	 * Type of the value, from #type_t.
	 * @see keyGetType(), keySetType(), keyIsBin()
	 */
	type_t         type;

	/**
	 * File-like mode control
	 * @see keyGetMode(), keySetMode()
	 */
	mode_t         mode;

	/**
	 * The value, which is a NULL terminated string or binary.
	 * @see keyGetString(), keyGetBinary(), keySetRaw() */
	void *         data;

	/**
	 * Size of the value, in bytes, including ending NULL.
	 * @see keyGetCommentSize(), keySetComment(), keyGetComment()
	 */
	size_t         dataSize;

	/**
	 * The name of the key.
	 * @see keySetName(), keySetName()
	 */
	char *         key;

	/**
	 * Size of the name, in bytes, including ending NULL.
	 * @see keyGetName(), keyGetNameSize(), keySetName()
	 */
	size_t         keySize;

	/**
	 * Some control and internal flags.
	 */
	keyflag_t      flags;

	/**
	 * In how many keysets the key resists.
	 * keySetName() and keyRemove() are only allowed if ksReference is 0.
	 * @see ksPop(), ksAppendKey(), ksAppend()
	 */
	size_t        ksReference;

	/**
	 * All the key's meta information.
	 */
	KeySet *      meta;
};




/**
 * The private KeySet structure.
 *
 * Its internal private attributes should not be accessed directly by regular
 * programs. Use the @ref keyset "KeySet access methods" instead.
 * Only a backend writer needs to have access to the private attributes of the
 * KeySet object which is defined as:
 * @code
typedef struct _KeySet KeySet;
 * @endcode
 *
 * @ingroup backend
 */
struct _KeySet {
	struct _Key **array;	/**<Array which holds the keys */

	size_t        size;	/**< Number of keys contained in the KeySet */
	size_t        rsize;	/**< Number of removed keys contained in the KeySet */
	size_t        alloc;	/**< Allocated size of array */

	struct _Key  *cursor;	/**< Internal cursor */
	size_t        current;	/**< Current position of cursor */
	ksflag_t      flags;	/**< Some control and internal flags. */
};


/**
 * The private Kdb Capability structure.
 *
 * Its internal private attributes should not be accessed directly by regular
 * programs. Use the @ref capability "KDBCapability access methods" instead.
 * Only a backend writer needs to have access to the private attributes of the
 * KeySet object which is defined as:
 * @code
typedef struct _KDBCap KDBCap;
 * @endcode
 * 
 * @see kdbGetCapabilty()
 * @see commandInfo() of the 'kdb info' command to see it in action
 * @ingroup capability
 */
struct _KDBCap {
        /* we'll point only to static strings. We won't allocate anything for each member. */
	const char *version;		/*!< Version of the library. */
	const char *name;		/*!< Name of backend being or that will be used. */
	const char *description;	/*!< Any text describing the backend. */
	const char *author;		/*!< The author of the backend. */
	const char *licence;		/*!< The licence of the backend,
				  because of BSD licence even commercial is allowed. */
	unsigned int onlyFullGet:1;	/*!< You can't get specific keys. */
	unsigned int onlyRemoveAll:1;	/*!< You can only remove all keys at once. */
	unsigned int onlyAddKeys:1;	/*!< When setting keys, they will be added. */
	unsigned int onlyFullSet:1;	/*!< All keys need to be in keyset when setting. */
	unsigned int onlySystem:1;	/*!< Only system namespace supported. */
	unsigned int onlyUser:1;	/*!< Only user namespace supported. */
	unsigned int noOwner:1;		/*!< The backend does not support a owner of keys. */
	unsigned int noValue:1;		/*!< No value is supported in the backend. */
	unsigned int noComment:1;	/*!< No comment is supported in the backend. */
	unsigned int noUID:1;		/*!< No uid is supported in the backend. */
	unsigned int noGID:1;		/*!< No gid is supported in the backend. */
	unsigned int noMode:1;		/*!< No mode is supported in the backend. */
	unsigned int noDir:1;		/*!< Directories are not supported in the backend. */
	unsigned int noATime:1;		/*!< Mode Time not supported. */
	unsigned int noMTime:1;		/*!< Modification Time not supported. */
	unsigned int noCTime:1;		/*!< Meta Info Change Time not supported. */
	unsigned int noRemove:1;	/*!< The backend does not support removing keys. */
	unsigned int noStat:1;		/*!< When getting keys they can't be stated. */
	unsigned int noMount:1;		/*!< Mount type not supported. */
	unsigned int noBinary:1;	/*!< Binary types not supported. */
	unsigned int noString:1;	/*!< String types not supported. */
	unsigned int noTypes:1;		/*!< Typing of keys is not supported. */
	unsigned int noError:1;		/*!< Don't expect errno to be set correctly. */
	unsigned int noLock:1;		/*!< Backend does not lock. */
	unsigned int noThread:1;	/*!< Backend uses global variables and is not threadsafe. */
};

/**
 * The structure which holds all information of a loaded backend.
 *
 * Its internal private attributes should not be accessed directly by regular
 * programs. Use the @ref capability "KDBCapability access methods" instead.
 * Only a backend writer needs to have access to the private attributes of the
 * KeySet object which is defined as:
 * @code
typedef struct _KDB KDB;
 * @endcode
 *
 * @see kdbOpen() and kdbClose() for external use
 * @ingroup backend
 */
struct _KDB {
	Trie *trie;		/*!< The pointer to the trie holding other backends.
		@see kdbhGetTrie() */

	KeySet *config;		/*!< This keyset contains configuration for the backend.
		Don't care about the absolute path, it may change when dynamically
		kdbMount() or because of another name under system/elektra/mountpoints. 
		The keys inside contain information like /path which path should be used
		to write configuration to or /host to which host packets should be send.
		@see kdbhGetConfig() */

	void *backendData;	/*!< A general pointer for any data backend needs.
		Thus backends are not allowed to have global variables for thread saftey,
		they must use this pointer.
		@see kdbhGetBackendData() */

	Key *mountpoint;	/*!< The mountpoint where the backend resides.
		The keyName() is the point where the backend was mounted.
		The keyValue() is the name of the backend without pre/postfix, e.g.
		filesys. */

	KDBCap *capability;	/*!< The capabilites this backend declares to have.
		@see kdbhGetCapability() */

	kdbLibHandle dlHandle;	/*!< The pointer to the datastructure to load a new backend. */

	/* These are the interfaces that must be implemented */

	kdbOpenPtr kdbOpen;	/*!< The pointer to kdbOpen_template() of the backend. */
	kdbClosePtr kdbClose;	/*!< The pointer to kdbClose_template() of the backend. */

	kdbGetPtr kdbGet;	/*!< The pointer to kdbGet_template() of the backend. */
	kdbSetPtr kdbSet;	/*!< The pointer to kdbSet_template() of the backend. */
};


/** The private trie structure.
 *
 * A trie is a data structure which can handle the longest prefix matching very
 * fast. This is exactly what needs to be done when using kdbGet() and kdbSet()
 * in a hierachy where backends are mounted - you need the backend mounted
 * closest to the parentKey.
 */
struct _Trie {
	struct _Trie *childs[MAX_UCHAR];/*!<  */
	char *text[MAX_UCHAR];		/*!<  */
	unsigned int textlen[MAX_UCHAR];/*!<  */
	void *value[MAX_UCHAR];		/*!<  */
	void *empty_value;		/*!< value for the empty string "" */
};


/** The private split structure.
 *
 * kdbSet() splits keysets. This structure contains arrays for
 * various information needed to process the keysets afterwards.
 */
struct _Split {
	size_t no;		/*!< Number of keysets */
	size_t alloc;		/*!< How large the arrays are allocated  */
	KeySet **keysets;	/*!< The keysets */
	KDB **handles;	/*!< The KDB for the keyset */
	Key **parents;		/*!< The parentkey for the keyset */
	int *syncbits;		/*!< Is there any key in there which need to be synced? */
	int *belowparents;	/*!< Is there any key in there which is below the parent? */
};


#ifdef __cplusplus
extern "C" {
#endif

/***************************************
 *
 * Functions which might be reintroduced
 *
 **************************************/

/*****************
 * Namespaces
 *****************/

/*
 * Elektra currently supported Key namespaces.
 *
 * @ingroup key
 * @see kdbGet(), keyGetNamespace(), keyNameGetNamespace()
 */
enum KeyNamespace {
	KEY_NS_SYSTEM=1,       /*!< The @p system keys */
	KEY_NS_USER=2          /*!< The @p user keys */
};

int keyGetNamespace(const Key *key);
int keyNameGetNamespace(const char *keyname);


/*****************
 * Misc Functions
 *****************/

keyswitch_t keyCompare(const Key *key1, const Key *key2);
ssize_t ksGetCommonParentName(const KeySet *ks,char *returnedCommonParent,
	size_t maxSize);



/*****************
 * Serialization
 *****************/

int keySerialize(const Key *key, void *serialized, size_t maxSize);
Key *keyCompose(const void *serialized);
Key *keyUnserialize(const void *serialized); 
size_t keyGetSerializedSize(const Key *key);

/*****************
 * Allocation
 *****************/

int ksNeedSort(const KeySet *ks);
int ksResize(KeySet *ks, size_t size);
size_t ksGetAlloc(const KeySet *ks);


/***************************************
 *
 * Not exported functions, for internal use only
 *
 **************************************/

KDB* kdbOpenBackend(const char *backendname, const char *mountpoint, KeySet *config);
int kdbCloseBackend(KDB *handle);

ssize_t keySetRaw(Key *key, const void *newBinary, size_t dataSize);

char *keyNameGetOneLevel(const char *keyname, size_t *size);

ssize_t keyNameGetRootNameSize(const char *keyname);
ssize_t keyNameGetBaseNameSize(const char *keyname);
ssize_t keyNameGetFullRootNameSize(const char *keyname);

int keyNameIsSystem(const char *keyname);
int keyNameIsUser(const char *keyname);

ssize_t keyGetRootNameSize(const Key *key);
ssize_t keyGetRootName(const Key *key, char *returned, size_t maxSize);

ssize_t keyGetFullRootNameSize(const Key *key);
ssize_t keyGetFullRootName(const Key *key, char *returned, size_t maxSize);

ssize_t keyGetParentName(const Key *key, char *returned, size_t maxSize);
ssize_t keyGetParentNameSize(const Key *key);

KDB *kdbGetBackend(KDB *handle, const Key *key);

int kdbCreateTrie(KDB *handle, KeySet *ks, OpenMapper mapper);
int kdbDelTrie(Trie *trie,CloseMapper close_backend);

Trie *createTrie(KeySet *ks, OpenMapper mapper);
Trie *delete_trie(Trie *trie, char *name, CloseMapper close_mapper);
Trie *insert_trie(Trie *trie, const char *name, const void *value);

void free_splitted_keysets(Split *keysets);
void init_splitted_keysets(Split *ret);
void resize_splitted_keysets(Split *ret);
Split *split_keyset(KDB *handle, KeySet *ks,
	Key *parentKey, unsigned long options);

int kdbiStrCaseCmp (const char *s1, const char *s2);
size_t kdbiStrLen(const char *s);

void *kdbiMalloc (size_t size);
void  kdbiFree (void *ptr);
char *kdbiStrDup (const char *s);
char *kdbiStrNDup (const char *s, size_t l);
int kdbiRealloc(void **buffer, size_t size);

ssize_t kdbbEncode(void *kdbbDecoded, size_t size, char *returned);
ssize_t kdbbDecode(char *kdbbEncoded, void *returned);

int kdbbNeedsUTF8Conversion(void);
int kdbbUTF8Engine(int direction, char **string, size_t *inputByteSize);

int kdbbEncodeChar(char c, char *buffer, size_t bufSize);
int kdbbDecodeChar(const char *from, char *into);

int kdbbFilenameToKeyName(const char *string, char *buffer, int bufSize);
ssize_t kdbbGetFullKeyName (KDB *handle, const char *forFilename, const Key *parentKey, Key *returned);
int kdbbKeyNameToRelativeFilename(const char *string, char *buffer, size_t bufSize);
ssize_t kdbbKeyCalcRelativeFilename(const Key *key,char *relativeFilename,size_t maxSize);
ssize_t kdbbGetFullFilename(KDB *handle, const Key *forKey,char *returned,size_t maxSize);

int keyInit(Key *key);
int keyClose(Key *key);
int keyClear(Key *key);

int ksInit(KeySet *ks);
int ksClose(KeySet *ks);
int ksClear(KeySet *ks);

/** Test a bit. @see set_bit(), clear_bit() */
#define test_bit(var,bit)            ((var) &   (bit))
/** Set a bit. @see clear_bit() */
#define set_bit(var,bit)             ((var) |=  (bit))
/** Clear a bit. @see set_bit() */
#define clear_bit(var,bit)           ((var) &= ~(bit))

#ifdef __cplusplus
}
#endif

#endif /* KDBPRIVATE_H */

#endif /* KDB_H first check */
