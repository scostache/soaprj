## HybFS Implementation ##

The implementation of HybFS consists of three modules as described in Figure 1: the user-space file system, a library comprising of an uniform interface to the metadata storage and an application that allows loading of multiple plug-ins for extracting metadata from files, getting statistics about tags for testing purposes and transferring files between two HybFS mount points, together with their metadata.
The plug-in support is represented through a generic interface that can be extended by each new plug-in sub-module. For further developing purposes, the library interface supports multiple instances of the back-end interface, called **virtual directory branches**, each one of them having its own database connection. However, for now, this can be useful only from the HybFS application.

> ![http://soaprj.googlecode.com/svn/wiki/hybfs_design.jpeg](http://soaprj.googlecode.com/svn/wiki/hybfs_design.jpeg)

  1. A file system operation is issued (think of create, rename, unlink, etc.) and is passed to the HybFS filesystem.
  1. The provided path is analyzed and validated by _the query parser_ and the results are packed in an internal representation used afterwards to issue operations to the database backend.
  1. The metadata from the main database is accesed and/or modified.
  1. The results are returned to the HybFS core.
  1. Based on the results obtained, and if needed, the core interface issues a series of operations to the underlying filesystem(s).

### Technologies ###

  1. **FUSE** (File system in user space) - an abstraction layer that allows a fully functional file system to be written in user space, and allows
> regular users to mount file systems. It provides a simple and efficient API for rapid development.

  1. **Sqlite3** - an Open Source embedded database library.
  1. **Boost** libraries

  1. C/C++ - for the filesystem implementation
  1. C++   - for the policy management framework


---


### Frontend ###

The front-end is represented by the HybFS file system interface. We implemented our file system with the FUSE toolkit for user-level file systems in Linux. In this way, we structured HybFS as a layer of extended-content around an already existent location on the file system. The HybFS file system application implements the common file system operations needed to provide basic functionality. Also, it is responsible for initializing the back-end interface for the mounted directory, parsing the paths and passing the results to it.
The abstract flow of a simplified request is described as follows:
  1. A file system operation is issued (e.g. create, rename, unlink) by a user application and it is passed by the VFS to the FUSE file system driver and then to the HybFS file system.
  1. In HybFS, the provided file path is parsed and the resulted queries and the real file path (if any) are packed in an internal representation used afterwards to issue operations to the metadata database.
  1. In the case of a tag operation or a query, the metadata from the database is accessed and, possibly, modified.
  1. The results are returned to the HybFS core.
  1. Based on the results obtained, and if needed, the HybFS interface passes the operation to the underlying file system.


---


### Backend ###

The backend module is represented by a library used to access the metadata database from the file system interface and our application. This provides a uniform interface to the query parsing and the database access internal methods, allowing the future development of other metadata indexing solutions.

For indexing the semantic attributes of the files, we use a relational database. This allows any search query to be directly mapped on a SQL query. The database is implemented using Sqlite, which is linked in the application as a library. The database contains three tables with information about the tagged files. The first table contains the file inode number, the file mode and the real path relative to the mounted directory. Also we keep all the tags assigned for a file in a separate field, for issuing search queries faster. The second table is for storing the tag and value pairs, together with a unique id and the last one keeps the association between a file id and a tag:value id. The last two tables can be used for tag statistics from the HybFS Control Application. For simple tags (that don't have a value), we assign the "null" value. The metadata directory is set in each mount point, with the name ".hybfs" . The main database is kept in the file ".hybfs\_main.db", thus all the tables are in the same file._


---


### HybFS Control Application ###

The HybFS Control application provides an interface to define tagging behaviors for different mounted directories. This will allow the user to tag automatically files based on their types and the existing supported tagging modules. For now we support tag extraction for MP3's and JPEG files.
The application allows specifing multiple HybFS mount points and for each mount point a different set of plugins can be loaded. For example if we have the /fs1 and /fs2 as the directories in which HybFS was mounted, we can load the MP3 plugin for /fs1 and both the MP3 and EXIF plugins for /fs2. When indexing a directory from /fs1 only the mp3 files are parsed and their information loaded into the appropriate database while from a directory from /fs2 both mp3 and picture files are processed.

The application can also be used for special file operations like copy and move. When files are copied between different HybFS mount points, their tags are also transfered between the two databases. Even if there wasn't any HybFS file system mounted for a specific location, that location can be defined as a mount point in the application and a new database is created in order to permit copy/move operations to/from the new defined location. This allows the transfer of files together with their additional tags. The operation needed for this implies the defining of the directory where the files will be copied as a mount point using the HybFS Control Application on both the source and the destination. When defining an additional directory as a mount point, the application will initialize the database storage for tagging information, if it is not defined already.