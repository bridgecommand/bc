// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_MOUNT_READER_H_INCLUDED__
#define __C_MOUNT_READER_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef __IRR_COMPILE_WITH_MOUNT_ARCHIVE_LOADER_

#include "IFileSystem.h"
#include "CFileList.h"

namespace irr
{
namespace io
{

	//! Archiveloader capable of loading MountPoint Archives
	class CArchiveLoaderMount : public IArchiveLoader
	{
	public:

		//! Constructor
		CArchiveLoaderMount(io::IFileSystem* fs);

		//! returns true if the file maybe is able to be loaded by this class
		//! based on the file extension (e.g. ".zip")
		virtual bool isALoadableFileFormat(const io::path& filename) const _IRR_OVERRIDE_;

		//! Check if the file might be loaded by this class
		/** Check might look into the file.
		\param file File handle to check.
		\return True if file seems to be loadable. */
		virtual bool isALoadableFileFormat(io::IReadFile* file) const _IRR_OVERRIDE_;

		//! Check to see if the loader can create archives of this type.
		/** Check based on the archive type.
		\param fileType The archive type to check.
		\return True if the archile loader supports this type, false if not */
		virtual bool isALoadableFileFormat(E_FILE_ARCHIVE_TYPE fileType) const _IRR_OVERRIDE_;

		//! Creates an archive from the filename
		/** \param file File handle to check.
		\return Pointer to newly created archive, or 0 upon error. */
		virtual IFileArchive* createArchive(const io::path& filename, bool ignoreCase, bool ignorePaths) const _IRR_OVERRIDE_;

		//! creates/loads an archive from the file.
		//! \return Pointer to the created archive. Returns 0 if loading failed.
		virtual IFileArchive* createArchive(io::IReadFile* file, bool ignoreCase, bool ignorePaths) const _IRR_OVERRIDE_;

	private:
		io::IFileSystem* FileSystem;
	};

	//! A File Archive which uses a mountpoint
	class CMountPointReader : public virtual IFileArchive, virtual CFileList
	{
	public:

		//! Constructor
		CMountPointReader(IFileSystem *parent, const io::path& basename,
				bool ignoreCase, bool ignorePaths);

		//! opens a file by index
		virtual IReadFile* createAndOpenFile(u32 index) _IRR_OVERRIDE_;

		//! opens a file by file name
		virtual IReadFile* createAndOpenFile(const io::path& filename) _IRR_OVERRIDE_;

		//! returns the list of files
		virtual const IFileList* getFileList() const _IRR_OVERRIDE_;

		//! get the class Type
		virtual E_FILE_ARCHIVE_TYPE getType() const _IRR_OVERRIDE_ { return EFAT_FOLDER; }

		//! return the name (id) of the file Archive
		virtual const io::path& getArchiveName() const _IRR_OVERRIDE_ {return Path;}

	private:

		core::array<io::path> RealFileNames;

		IFileSystem *Parent;
		void buildDirectory();
	};
} // io
} // irr

#endif // __IRR_COMPILE_WITH_MOUNT_ARCHIVE_LOADER_
#endif // __C_MOUNT_READER_H_INCLUDED__
