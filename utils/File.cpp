#include <cassert>

#include "utils/File.h"
#include "utils/Buffer.h"

File::File() :
	path_(),
	file_(nullptr),
	stat_({})
{
}

File::File(const std::string& file):
	File(file, File::O_RDONLY)
{
}

File::File(const std::string& file, OpenMode openmode):
	path_(file),
	stat_()
{
	file_ = fopen(file.c_str(), OpenModeToString(openmode).c_str());
}

File::~File()
{
	Close();
}

bool File::Open(const std::string& file, OpenMode openmode)
{
	if (file_)
	{
		return false;
	}
	file_ = fopen(file.c_str(), OpenModeToString(openmode).c_str());

	return file_ != nullptr;
}

void File::Close()
{
	if (file_)
	{
		fclose(file_);
		file_ = nullptr;
	}
}

ssize_t File::Read(char* buffer, size_t length)
{
	if (!file_)
	{
		return -1;
	}
	ssize_t result = fread(buffer, sizeof (char), length, file_);

	assert(result >= 0);

	return result;
}

ssize_t File::Read(Buffer* buffer)
{
	if (!buffer)
	{
		return -1;
	}

	buffer->AdjustBuffer();
	ssize_t result = Read(buffer->WriteBegin(), buffer->WritableLength());
	buffer->AddWriteIndex(result);

	return result;
}

ssize_t File::Write(const char* buffer, size_t length)
{
	if (!file_)
	{
		return -1;
	}

	ssize_t result = fwrite(buffer, 1, length, file_);
	assert(result >= 0);

	return result;
}

ssize_t File::Write(const Buffer* buffer)
{
	if (!buffer)
	{
		return -1;
	}
	ssize_t result = Write(buffer->ReadBegin(), buffer->ReadableLength());
	if (result <= 0)
	{
		return result;
	}
	return result;
}

size_t File::GetFileSize()
{
	if (stat_.st_size == 0)
	{
		stat(path_.c_str(), &stat_);
	}

	return stat_.st_size;
}

void File::Flush()
{
	if (file_)
	{
		fflush(file_);
	}
};

std::string File::OpenModeToString(OpenMode openmode)
{
	std::string mode;
	switch (openmode)
	{
	case File::O_RDONLY:
		mode = "rb";
		break;
	case File::O_WRONLY:
		mode = "wb";
		break;
	case File::O_APPEND:
		mode = "ab";
		break;
	case File::O_RDWR:
		mode = "rb+";
		break;
	default:
		break;
	}
	return mode;
}

const std::string& File::GetPath() const
{
	return path_;
}

bool File::IsOpened() const
{
	return file_ != nullptr;
}

bool File::ReadFileAsString(const std::string& file_path, std::string& content)
{
	File file(file_path);
	if (!file.IsOpened())
	{
		return false;
	}
	size_t file_size = file.GetFileSize();
	size_t read_size = 0;
	content.reserve(file_size);

	char buffer[1024];
	while (read_size < file_size)
	{
		ssize_t read_length = file.Read(buffer, sizeof buffer);
		if (read_length > 0)
		{
			read_size += read_length;
			content.append(buffer, read_length);
		}
		else if (read_length == 0)
		{
			break;
		}
		else
		{
			return false;
		}
	}
	return true;

}
