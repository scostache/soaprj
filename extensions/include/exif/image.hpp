// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2008 Andreas Huggel <ahuggel@gmx.net>
 *
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*!
  @file    image.hpp
  @brief   Class JpegImage to access JPEG images
  @version $Rev: 1404 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @author  Brad Schick (brad)
           <a href="mailto:brad@robotbattle.com">brad@robotbattle.com</a>
  @date    09-Jan-04, ahu: created<BR>
           11-Feb-04, ahu: isolated as a component<BR>
           19-Jul-04, brad: revamped to be more flexible and support IPTC<BR>
           15-Jan-05, brad: inside-out design changes
 */
#ifndef IMAGE_HPP_
#define IMAGE_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"
#include "basicio.hpp"
#include "exif.hpp"
#include "iptc.hpp"
#include "xmp.hpp"

// + standard includes
#include <string>
#include <vector>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// type definitions

    //! Container for binary image
    typedef std::vector<byte> Blob;

// *****************************************************************************
// class definitions

    //! Supported image formats
    namespace ImageType {
        const int none = 0;         //!< Not an image
    }

    /*!
      @brief Abstract base class defining the interface for an image. This is
         the top-level interface to the Exiv2 library.

      Image has containers to store image metadata and subclasses implement
      read and save metadata from and to specific image formats.<BR>
      Most client apps will obtain an Image instance by calling a static
      ImageFactory method. The Image class can then be used to to read, write,
      and save metadata.
     */
    class Image {
    public:
        //! Image auto_ptr type
        typedef std::auto_ptr<Image> AutoPtr;

        //! @name Creators
        //@{
        /*!
          @brief Constructor taking the image type, a bitmap of the supported
              metadata types and an auto-pointer that owns an IO instance. 
              See subclass constructor doc.
         */
        Image(int              imageType, 
              uint16_t         supportedMetadata,
              BasicIo::AutoPtr io);
        //! Virtual Destructor
        virtual ~Image() {}
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Read all metadata supported by a specific image format from the
              image. Before this method is called, the image metadata will be
              cleared.

          This method returns success even if no metadata is found in the
          image. Callers must therefore check the size of individual metadata
          types before accessing the data.

          @throw Error if opening or reading of the file fails or the image
              data is not valid (does not look like data of the specific image
              type).
         */
        virtual void readMetadata() =0;
        /*!
          @brief Write metadata back to the image.

          All existing metadata sections in the image are either created,
          replaced, or erased. If values for a given metadata type have been
          assigned, a section for that metadata type will either be created or
          replaced. If no values have been assigned to a given metadata type,
          any exists section for that metadata type will be removed from the
          image.

          @throw Error if the operation fails
         */
        virtual void writeMetadata() =0;
        /*!
          @brief Assign new Exif data. The new Exif data is not written
              to the image until the writeMetadata() method is called.
          @param exifData An ExifData instance holding Exif data to be copied
         */
        virtual void setExifData(const ExifData& exifData);
        /*!
          @brief Erase any buffered Exif data. Exif data is not removed from
              the actual image until the writeMetadata() method is called.
         */
        virtual void clearExifData();
        /*!
          @brief Assign new IPTC data. The new IPTC data is not written
              to the image until the writeMetadata() method is called.
          @param iptcData An IptcData instance holding IPTC data to be copied
         */
        virtual void setIptcData(const IptcData& iptcData);
        /*!
          @brief Erase any buffered IPTC data. IPTC data is not removed from
              the actual image until the writeMetadata() method is called.
         */
        virtual void clearIptcData();
        /*!
          @brief Assign a raw XMP packet. The new XMP packet is not written
              to the image until the writeMetadata() method is called.

          Subsequent calls to writeMetadata() write the XMP packet from 
          the buffered raw XMP packet rather than from buffered parsed XMP 
          data. In order to write from parsed XMP data again, use
          either writeXmpFromPacket(false) or setXmpData().

          @param xmpPacket A string containing the raw XMP packet.
         */
        virtual void setXmpPacket(const std::string& xmpPacket);
        /*!
          @brief Erase the buffered XMP packet. XMP data is not removed from
              the actual image until the writeMetadata() method is called.

          This has the same effect as clearXmpData() but operates on the 
          buffered raw XMP packet only, not the parsed XMP data.

          Subsequent calls to writeMetadata() write the XMP packet from 
          the buffered raw XMP packet rather than from buffered parsed XMP
          data. In order to write from parsed XMP data again, use
          either writeXmpFromPacket(false) or setXmpData().
         */
        virtual void clearXmpPacket();
        /*!
          @brief Assign new XMP data. The new XMP data is not written
              to the image until the writeMetadata() method is called.

          Subsequent calls to writeMetadata() encode the XMP data to
          a raw XMP packet and write the newly encoded packet to the image.
          In the process, the buffered raw XMP packet is updated. 
          In order to write directly from the raw XMP packet, use
          writeXmpFromPacket(true) or setXmpPacket().

          @param xmpData An XmpData instance holding XMP data to be copied
         */
        virtual void setXmpData(const XmpData& xmpData);
        /*!
          @brief Erase any buffered XMP data. XMP data is not removed from
              the actual image until the writeMetadata() method is called.

          This has the same effect as clearXmpPacket() but operates on the 
          buffered parsed XMP data.

          Subsequent calls to writeMetadata() encode the XMP data to
          a raw XMP packet and write the newly encoded packet to the image.
          In the process, the buffered raw XMP packet is updated.
          In order to write directly from the raw XMP packet, use
          writeXmpFromPacket(true) or setXmpPacket().
         */
        virtual void clearXmpData();
        /*!
          @brief Set the image comment. The new comment is not written
              to the image until the writeMetadata() method is called.
          @param comment String containing comment.
         */
        virtual void setComment(const std::string& comment);
        /*!
          @brief Erase any buffered comment. Comment is not removed
              from the actual image until the writeMetadata() method is called.
         */
        virtual void clearComment();
        /*!
          @brief Copy all existing metadata from source Image. The data is
              copied into internal buffers and is not written to the image
              until the writeMetadata() method is called.
          @param image Metadata source. All metadata types are copied.
         */
        virtual void setMetadata(const Image& image);
        /*!
          @brief Erase all buffered metadata. Metadata is not removed
              from the actual image until the writeMetadata() method is called.
         */
        virtual void clearMetadata();
        /*!
          @brief Returns an ExifData instance containing currently buffered
              Exif data.

          The contained Exif data may have been read from the image by
          a previous call to readMetadata() or added directly. The Exif
          data in the returned instance will be written to the image when
          writeMetadata() is called.

          @return modifiable ExifData instance containing Exif values
         */
        virtual ExifData& exifData() { return exifData_; }
        /*!
          @brief Returns an IptcData instance containing currently buffered
              IPTC data.

          The contained IPTC data may have been read from the image by
          a previous call to readMetadata() or added directly. The IPTC
          data in the returned instance will be written to the image when
          writeMetadata() is called.

          @return modifiable IptcData instance containing IPTC values
         */
        virtual IptcData& iptcData() { return iptcData_; }
        /*!
          @brief Returns an XmpData instance containing currently buffered
              XMP data.

          The contained XMP data may have been read from the image by
          a previous call to readMetadata() or added directly. The XMP
          data in the returned instance will be written to the image when
          writeMetadata() is called.

          @return modifiable XmpData instance containing XMP values
         */
        virtual XmpData& xmpData() { return xmpData_; }
        /*!
          @brief Return a modifiable reference to the raw XMP packet.
         */
        virtual std::string& xmpPacket() { return xmpPacket_; }
        /*!
          @brief Determine the source when writing XMP.

          Depending on the setting of this flag, writeMetadata() writes
          XMP from the buffered raw XMP packet or from parsed XMP data.
          The default is to write from parsed XMP data. The switch is also
          set by all functions to set and clear the buffered raw XMP packet
          and parsed XMP data, so using this function should usually not be 
          necessary.

          If %Exiv2 was compiled without XMP support, the default for this 
          flag is true and it will never be changed in order to preserve 
          access to the raw XMP packet.
         */
        void writeXmpFromPacket(bool flag);
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Check if the Image instance is valid. Use after object
              construction.
          @return true if the Image is in a valid state.
         */
        bool good() const;
        /*!
          @brief Return the MIME type of the image.

          @note For each supported image format, the library knows only one MIME
          type.  This may not be the most specific MIME type for that format. In
          particular, several RAW formats are variants of the TIFF format with
          the same magic as TIFF itself. Class TiffImage handles most of them
          and thus they all have MIME type "image/tiff", although a more
          specific MIME type may exist (e.g., "image/x-nikon-nef").
         */
        virtual std::string mimeType() const =0;
        /*!
          @brief Return the pixel width of the image.
         */
        virtual int pixelWidth() const { return pixelWidth_; }
        /*!
          @brief Return the pixel height of the image.
         */
        virtual int pixelHeight() const { return pixelHeight_; }
        /*!
          @brief Returns an ExifData instance containing currently buffered
              Exif data.

          The Exif data may have been read from the image by
          a previous call to readMetadata() or added directly. The Exif
          data in the returned instance will be written to the image when
          writeMetadata() is called.

          @return read only ExifData instance containing Exif values
         */
        virtual const ExifData& exifData() const { return exifData_; }
        /*!
          @brief Returns an IptcData instance containing currently buffered
              IPTC data.

          The contained IPTC data may have been read from the image by
          a previous call to readMetadata() or added directly. The IPTC
          data in the returned instance will be written to the image when
          writeMetadata() is called.

          @return modifiable IptcData instance containing IPTC values
         */
        virtual const IptcData& iptcData() const { return iptcData_; }
        /*!
          @brief Returns an XmpData instance containing currently buffered
              XMP data.

          The contained XMP data may have been read from the image by
          a previous call to readMetadata() or added directly. The XMP
          data in the returned instance will be written to the image when
          writeMetadata() is called.

          @return modifiable XmpData instance containing XMP values
         */
        virtual const XmpData& xmpData() const { return xmpData_; }
        /*!
          @brief Return a copy of the image comment. May be an empty string.
         */
        virtual std::string comment() const { return comment_; }
        /*!
          @brief Return the raw XMP packet as a string.
         */
        virtual const std::string& xmpPacket() const { return xmpPacket_; }
        /*!
          @brief Return a reference to the BasicIo instance being used for Io.

          This refence is particularly useful to reading the results of
          operations on a MemIo instance. For example after metadata has
          been modified and the writeMetadata() method has been called,
          this method can be used to get access to the modified image.

          @return BasicIo instance that can be used to read or write image
             data directly.
          @note If the returned BasicIo is used to write to the image, the
             Image class will not see those changes until the readMetadata()
             method is called.
         */
        virtual BasicIo& io() const { return *io_; }
        /*!
          @brief Returns the access mode, i.e., the metadata functions, which
             this image supports for the metadata type \em metadataId.
          @param metadataId The metadata identifier.
          @return Access mode for the requested image type and metadata identifier.
         */
        AccessMode checkMode(MetadataId metadataId) const;
        /*!
          @brief Check if image supports a particular type of metadata.
             This method is deprecated. Use checkMode() instead.
         */
        bool supportsMetadata(MetadataId metadataId) const;
        //! Return the flag indicating the source when writing XMP metadata.
        bool writeXmpFromPacket() const { return writeXmpFromPacket_; }
        //@}

    protected:
        // DATA
        BasicIo::AutoPtr  io_;                //!< Image data IO pointer
        ExifData          exifData_;          //!< Exif data container
        IptcData          iptcData_;          //!< IPTC data container
        XmpData           xmpData_;           //!< XMP data container
        std::string       comment_;           //!< User comment
        std::string       xmpPacket_;         //!< XMP packet
        int               pixelWidth_;        //!< image pixel width
        int               pixelHeight_;       //!< image pixel height

    private:
        //! @name NOT implemented
        //@{
        //! Copy constructor
        Image(const Image& rhs);
        //! Assignment operator
        Image& operator=(const Image& rhs);
        //@}

        // DATA
        const int         imageType_;         //!< Image type
        const uint16_t    supportedMetadata_; //!< Bitmap with all supported metadata types
        bool              writeXmpFromPacket_;//!< Determines the source when writing XMP

    }; // class Image

    //! Type for function pointer that creates new Image instances
    typedef Image::AutoPtr (*NewInstanceFct)(BasicIo::AutoPtr io, bool create);
    //! Type for function pointer that checks image types
    typedef bool (*IsThisTypeFct)(BasicIo& iIo, bool advance);

    /*!
      @brief Returns an Image instance of the specified type.

      The factory is implemented as a static class.
    */
    class ImageFactory {
        friend bool Image::good() const;
    public:
        /*!
          @brief Create an Image subclass of the appropriate type by reading
              the specified file. %Image type is derived from the file
              contents.
          @param  path %Image file. The contents of the file are tested to
              determine the image type. File extension is ignored.
          @return An auto-pointer that owns an Image instance whose type
              matches that of the file.
          @throw Error If opening the file fails or it contains data of an
              unknown image type.
         */
        static Image::AutoPtr open(const std::string& path);
        /*!
          @brief Create an Image subclass of the appropriate type by reading
              the provided memory. %Image type is derived from the memory
              contents.
          @param data Pointer to a data buffer containing an image. The contents
              of the memory are tested to determine the image type.
          @param size Number of bytes pointed to by \em data.
          @return An auto-pointer that owns an Image instance whose type
              matches that of the data buffer.
          @throw Error If the memory contains data of an unknown image type.
         */
        static Image::AutoPtr open(const byte* data, long size);
        /*!
          @brief Create an Image subclass of the appropriate type by reading
              the provided BasicIo instance. %Image type is derived from the
              data provided by \em io. The passed in \em io instance is
              (re)opened by this method.
          @param io An auto-pointer that owns a BasicIo instance that provides
              image data. The contents of the image data are tested to determine
              the type.
          @note This method takes ownership of the passed
              in BasicIo instance through the auto-pointer. Callers should not
              continue to use the BasicIo instance after it is passed to this method.
              Use the Image::io() method to get a temporary reference.
          @return An auto-pointer that owns an Image instance whose type
              matches that of the \em io data. If no image type could be
              determined, the pointer is 0.
          @throw Error If opening the BasicIo fails
         */
        static Image::AutoPtr open(BasicIo::AutoPtr io);
        /*!
          @brief Create an Image subclass of the requested type by creating a
              new image file. If the file already exists, it will be overwritten.
          @param type Type of the image to be created.
          @param path %Image file to create. File extension is ignored.
          @return An auto-pointer that owns an Image instance of the requested
              type.
          @throw Error If the image type is not supported.
         */
        static Image::AutoPtr create(int type, const std::string& path);
        /*!
          @brief Create an Image subclass of the requested type by creating a
              new image in memory.
          @param type Type of the image to be created.
          @return An auto-pointer that owns an Image instance of the requested
              type.
          @throw Error If the image type is not supported
         */
        static Image::AutoPtr create(int type);
        /*!
          @brief Create an Image subclass of the requested type by writing a
              new image to a BasicIo instance. If the BasicIo instance already
              contains data, it will be overwritten.
          @param type Type of the image to be created.
          @param io An auto-pointer that owns a BasicIo instance that will
              be written to when creating a new image.
          @note This method takes ownership of the passed in BasicIo instance
              through the auto-pointer. Callers should not continue to use the
              BasicIo instance after it is passed to this method.  Use the
              Image::io() method to get a temporary reference.
          @return An auto-pointer that owns an Image instance of the requested
              type. If the image type is not supported, the pointer is 0.
         */
        static Image::AutoPtr create(int type, BasicIo::AutoPtr io);
        /*!
          @brief Returns the image type of the provided file.
          @param path %Image file. The contents of the file are tested to
              determine the image type. File extension is ignored.
          @return %Image type or Image::none if the type is not recognized.
         */
        static int getType(const std::string& path);
        /*!
          @brief Returns the image type of the provided data buffer.
          @param data Pointer to a data buffer containing an image. The contents
              of the memory are tested to determine the image type.
          @param size Number of bytes pointed to by \em data.
          @return %Image type or Image::none if the type is not recognized.
         */
        static int getType(const byte* data, long size);
        /*!
          @brief Returns the image type of data provided by a BasicIo instance.
              The passed in \em io instance is (re)opened by this method.
          @param io A BasicIo instance that provides image data. The contents
              of the image data are tested to determine the type.
          @return %Image type or Image::none if the type is not recognized.
         */
        static int getType(BasicIo& io);
        /*!
          @brief Returns the access mode or supported metadata functions for an
              image type and a metadata type.
          @param type       The image type.
          @param metadataId The metadata identifier.
          @return Access mode for the requested image type and metadata identifier.
          @throw Error(13) if the image type is not supported.
         */
        static AccessMode checkMode(int type, MetadataId metadataId);
        /*!
          @brief Determine if the content of \em io is an image of \em type.

          The \em advance flag determines if the read position in the
          stream is moved (see below). This applies only if the type
          matches and the function returns true. If the type does not
          match, the stream position is not changed. However, if
          reading from the stream fails, the stream position is
          undefined. Consult the stream state to obtain more
          information in this case.

          @param type Type of the image.
          @param io BasicIo instance to read from.
          @param advance Flag indicating whether the position of the io
              should be advanced by the number of characters read to
              analyse the data (true) or left at its original
              position (false). This applies only if the type matches.
          @return  true  if the data matches the type of this class;<BR>
                   false if the data does not match
        */
        static bool checkType(int type, BasicIo& io, bool advance);

    private:
        //! Struct for storing image types and function pointers.
        struct Registry {
            //! Comparison operator to compare a Registry structure with an image type
            bool operator==(const int& imageType) const;

            // DATA
            int            imageType_;
            NewInstanceFct newInstance_;
            IsThisTypeFct  isThisType_;
            AccessMode     exifSupport_;
            AccessMode     iptcSupport_;
            AccessMode     xmpSupport_;
            AccessMode     commentSupport_;
        };

        //! @name Creators
        //@{
        //! Prevent construction: not implemented.
        ImageFactory();
        //! Prevent copy construction: not implemented.
        ImageFactory(const ImageFactory& rhs);
        //@}

        // DATA
        //! List of image types, creation functions and access modes
        static const Registry registry_[];
    }; // class ImageFactory

    //! Helper class modelling the TIFF header structure.
    class TiffHeader {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Default constructor. Optionally sets the byte order
                 (default: little endian).
         */
        explicit TiffHeader(ByteOrder byteOrder =littleEndian);
        //@}

        //! @name Manipulators
        //@{
        //! Read the TIFF header from a data buffer. Returns 0 if successful.
        int read(const byte* buf);
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Write a standard TIFF header into buf as a data string, return
                 number of bytes copied.

          Only the byte order of the TIFF header varies, the values written for
          offset and tag are constant, i.e., independent of the values possibly
          read before a call to this function. The value 0x00000008 is written
          for the offset, tag is set to 0x002a.

          @param buf The data buffer to write to.
          @return The number of bytes written.
         */
        long copy(byte* buf) const;
        //! Return the size of the TIFF header in bytes.
        long size() const { return 8; }
        //! Return the byte order (little or big endian).
        ByteOrder byteOrder() const { return byteOrder_; }
        //! Return the tag value.
        uint16_t tag() const { return tag_; }
        /*!
          @brief Return the offset to IFD0 from the start of the TIFF header.
                 The offset is 0x00000008 if IFD0 begins immediately after the
                 TIFF header.
         */
        uint32_t offset() const { return offset_; }
        //@}

    private:
        ByteOrder byteOrder_;
        uint16_t tag_;
        uint32_t offset_;

    }; // class TiffHeader

// *****************************************************************************
// template, inline and free functions

    //! Append \em len bytes pointed to by \em buf to \em blob.
    void append(Blob& blob, const byte* buf, uint32_t len);

}                                       // namespace Exiv2

#endif                                  // #ifndef IMAGE_HPP_
