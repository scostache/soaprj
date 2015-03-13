


---


# HybFS meta-information #

Each file can be described by using one or multiple tags. A tag represents an additional attribute set by the user. The tags can be simple, or have an associated value.

**Simple tags** are specific keywords added to describe a file. The **attribute-value** tags  are used to granulate even more the description. In this case, the attribute represents the criteria of description and the value is the subcategory.
Ex: _the file_ **photo1.jpg** _can have as simple tags_ **holiday** _and_ **ski** _, and as attribute-value tags_ **year:2005** _,_ **author:stefania** _and_ **type:jpeg**.

Also, a file can have a general tag (without a value associated) and some other files can have the same tag, but with some values. The general tag will have by default an associated value of **null**.
Ex: suppose we have the file **fruits.jpg** and we assign the tag **apple**, because it contains some nice representations of this kind of fruit. Also, we
can have a **receipt.doc** with the tag  **apple:sweet**, because it describes some apple pie receipt and it uses sweet apples.

To find the files from the above example, we can specify a path that can be a combination of multiple tags with or without associated values.

In order for the user to have also a hierarchic view, we can see the file path as a special value for the tag **path**.


---


# HybFS semantics #

The navigation in HybFS resembles the navigation through a normal hierarchic file system, except the path is in fact a **query** on the tag information associated with the files. From the application point of view, simple file system operations like open/read/write/close remain unchanged when using HybFS.

## Query syntax ##

The queries will support the following logic operators:
  * **+** - logic **AND**
  * **|** - logic **OR**
  * **!** - logic **NOT**

Beside logic operators, brackets can be used for more complicated queries.

Ex: **ls '((type:photo + year:2005) + (!ski + !snowboard))'** - will display all files that have the tag **type:photo** and **year:2005**, but they don't have **ski** and **snowboard** tags.

A query must be supplied by using the sepparators '(' and ')', otherwise it will be considered illegal. Also, we can provide a normal navigation
through the original hierarchy, by using the special virtual directory **path:/** .

## HybFS operations ##

Our objective here is to keep the same interface as for the hierarchic file system operations and in the same time to seamlessly add semantic operations.

Concerning **navigation**, the system will be **2-Dimensional**:
  * the current path from the hierarchical file system and/or:
  * the current virtual folder, which retains the current query


### File listing ###

When listing the root directory, we will see all the tags and values from the file system, and the special directory **path:/**:
```
mount$ ls -l

(abstract)       (abstract:green)  path:/     (picture:autumn)    (year)
(abstract:blue)  (abstract:red)    (picture)  (picture:fractals)  (year:2008)
```

All files will be listed based on a query (if a query exists). If there are no semantic queries and the path is specified by using the special directory
**path:/**, the behavior will be the same as in a hierarchic file system. If there is a path build from queries and a real path (specified by the tag **path:**), then all the files that match the desired pattern and have that real path will be listed.

Example:
Suppose that we want to list all files from the folder **autumn** that have the tag **year:2008**:
```
mount$ ls 'path:/autumn/(year:2008)'

IMG_6577.JPG  IMG_6583.JPG  IMG_6590.JPG  IMG_6684.JPG      toamna1.jpg
IMG_6578.JPG  IMG_6584.JPG  IMG_6639.JPG  IMG_6685.JPG
IMG_6579.JPG  IMG_6587.JPG  IMG_6662.JPG  (picture:autumn)
IMG_6581.JPG  IMG_6589.JPG  IMG_6683.JPG  poza_mica.JPG
```

One of the possible drawbacks of full semantic file systems is that the user doesn't receive any suggestions when navigating through the files and it has to know what is looking for. This was solved by adding virtual folders to refine the results of the navigation. In our example, when we look for files that have been taken in **2008** and are in the real folder **/autumn/**, but we may not know that these files have other tags also.By adding the other tags as virtual directories, we can refine our search.

Example:
Suppose that we want to list all files that have the tag **picture:fractals** but they are not of type **abstract:red**:
```
mount$ ls '(picture:fractals+ !abstract:red)'

(abstract:blue)          Abstract-Blue-31272.jpg  Abstract-Green-31276.jpg
Abstract-Blue-22800.jpg  Abstract-Blue-31514.jpg  Abstract-Green-31276.png
Abstract-Blue-30666.jpg  (abstract:green)
```

**abstract:blue** and **abstract:green** are virtual directories and **Abstract-**.jpg**are the resulted files. Even if the listed tags weren't included in the query, they are still listed, to refine the search (maybe the user wants the files that are fractal pictures, not red, but blue, or green).**


### File rename ###

The renaming of files keeps the usual syntax and it can be used also for operations on tags.

Example:
To rename the file **photo1.jpg** as **first\_photo.jpg**, one can identify in a unique way the source and the destination by specifing the values for the special tag _path_ :
```
mount$ mv 'path:/directory/photo1.jpg' 'path:/directory/path/first_photo.jpg'
```
or it can specify a set of files determined by combinations of tags, to be moved in another directory, and other tags to be assignated:
```
mount$ mv '(ski + holiday)/*' '/path:/pictures/alps/(alps)'
```
Here, all the tags for the files that match the pattern **ski+holiday** will be replaced with the tag **alps** and will be moved in the directory **/pictures/alps/**.


### Replace tags ###

This operation is done using the rename operation, but this time applied to tags. With this operation, all the tags from the files depicted by the source query are removed, new tags being added by specifying them in the destination query. This will tell which are the tags that will replace the original ones. Also, the query that specifies the new tags to be assigned must be based on conjunctions only. This is happening because a different sintax will be too ambiguous.

Example:
For all the files that have the **type:photo** and **author:stefania** tags defined, we want to delete all the other tags and transform the **type:photo** into **type:jpg**. This operation can be done as following:
```
mount$ mv '(type:photo + author:stefania)/*' '(type:jpg)'
```

Remember, if there is no real path specified, with the "/path:/" prefix, no real rename of the files will be performed. However, if we _are_ in the directory **path:/** when the rename will be performed, then the files will be also renamed (moved).


### File remove ###

In order to actually remove a file, you can use a query to point out the files based on tag information. All files resulted from the query will be deleted along with their tag information from the DB.

Example:
To delete all the files from the mount point that have the tags **abstract:red** _or_ **abstract:green**:
```
mount$  rm '(abstract:green | abstract:red)'/*
```

### File creation ###

If someone wants to create a file, than the real path with the prefix **/path:/** must be specified. Also, to add tags at creation time, they must be specified in (only) a conjunction query.

Example:
```
mount/path:$ touch '(stories:winter)'/winter_story
```

### Add/Remove tags ###

This can be done by using the rename operation. Also, the tags can be specified at creation.
Example:
To add the tags _alps_ and _ski_ to the file /directory/path/photo1.jpg, and to keep the previous tags in the same time :
```
mount$ mv 'path:/directory/path/photo1.jpg' '(| alps+ski)'
```
To remove the tag _alps_ for the file /directory/path/photo1.jpg, but to keep the other tags:
```
mount$ mv 'path:/directory/path/photo1.jpg' '(!alps)'
```

Also notice the syntax for adding/removing tags: to add tags you must specify the append (disjunction) operator **|**, and to remove them, the negation operator **!**.

Remember to specify an absolute query (just do the operation in the root directory of the mount point), because if it's relative, it will be appended to the current query! Sometimes this can result in an invalid query!



---


# Known limitations and issues #

  1. Because a complex query has a certain syntax, the following characters are reserved for describing the possible queries or the tag-associated values:
    * **(**
    * **)**
    * **:**
    * **+**
    * **|**
    * **!**
Therefore, HybFS does not permit file names containing these characters, due to the complexity of queries.

  1. File copy - We need a special tool to copy the file from one directory to another, because we also need to copy the metadata
  1. The dual operation - on tags and files - raises issues related to tag remove and replace operations.
  1. Query navigation - if a user navigates from a query to another, it will be seen as a query conjunction.
  1. When expecting a descriptive error from an operation, you will get instead a weird one.
