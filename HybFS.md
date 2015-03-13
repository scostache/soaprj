# Project overview #

HybFS is an _almost_ semantic file system on Linux that can be overlaid on other file systems and provide multiple organizational views of the same original hierarchy of files, without altering the interface for the existing applications.

In a semantic file system, the navigation is based on the additional metadata associated with files, therefore offering multiple views of the same files. In a hierarchical file system, there is only one organizational view of the data, and a file can be identified in a unique way using this view. We mentioned that HybFS is an _almost_ semantic file system, because it actually represents a compromise between the hierarchic and semantic file systems. The user can organize files in a hierarchic way, but in the same time can assign tags, or keywords, to the files. This file system comes with two view perspectives to the user:
  1. Hierarchic view of the files
  1. Semantic view of the files


## HybFS Features ##
With HybFS one can have access to tag information from the interface of a file system and can specify paths for multiple directories and what plugins to use for parsing, by using a separate application. As an example, you can use the EXIF plugin for the images directory and the MP3 plugin for the music directory. However, the file indexing is not automated, for now, the user must specify the file or the directory to be parsed

### Tags ###

A tag can describe some content-related information of the file, when they are extracted directly from the file content, or a more general information about it. The tags can be simple, or can have an associated value, that is  used to granulate even more the description. In this case, the attribute represents the criteria of description and the value - the subcategory. If the tag has no associated value, it will be associated with a special value, **null**.  A user can create and assign many tags to a file or groups of files containing other tags.

### Queries ###

Any combination of multiple tags, and/or tag-value pairs is seen as a virtual directory that has the same name as the search query itself. We call it  a virtual directory because it only exist as an abstraction exported by HybFS and created at runtime. An entry in the virtual directory can be seen as a symbolic link to a file that match the current search pattern, or it can be a virtual directory that describes the other tags, or **tag:value** pairs assigned to the file entry, for further refining the query. In order for the user to have a hierarchic view, we define a directory with the name **"path:"** for the original mount point. This is desired for a better granularity. As an example, one can organize the pictures in directories based on the location where they were taken and refine the search with the aid of tags by date, people names, camera model and other.

### Operations ###

The common file system operations are the same as in a normal file system but without support for symbolic and hard links yet. HybFS also supports operations like add, replace or remove for the file tags. The navigation in HybFS resembles the navigation through a normal hierarchic file system, except that the file path can be also  based on the tag information associated with the files. From the application point of view, the result of a query is seen as a directory.


## Resources (or what you need...) ##
  * FUSE - We decided that for our file system implementation we should use the FUSE API, because it allows a rapid development. Also, we want to allow regular users to mount and configure our file system.
  * Sqlite3 - For storing the associated meta-data we use embedded database engine that can be linked directly into an application.
  * Boost libraries - Offer a large set of functionalities and we will keep them for future use.
  * Doxygen - for documentation