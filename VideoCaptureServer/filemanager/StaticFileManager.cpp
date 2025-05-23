#include "StaticFileManager.hpp"

oatpp::String getFileExtension(const oatpp::String& filename)
{
	int i;

	for (i = filename->size() - 1; i > 0; --i)
	{
		if (filename->data()[i] == '.')
		{
			break;
		}
	}

	if (i < 0 || i == filename->size() - 1)
	{
		return nullptr;
	}

	return oatpp::String((const char*)&filename->data()[i + 1], filename->size() - i - 1);
}

oatpp::String formatText(const char* text, ...)
{
	char buffer[8193];
	va_list args;
	va_start(args, text);
	vsnprintf(buffer, 8192, text, args);
	va_end(args);
	return oatpp::String(buffer);
}

oatpp::String getMimeType(const oatpp::String& filename)
{
	oatpp::String ext = getFileExtension(filename);

	if (ext == "m3u8")
	{
		return "application/x-mpegURL";
	}
	if (ext == "mp4")
	{
		return "video/mp4";
	}
	if (ext == "ts")
	{
		return "video/MP2T";
	}
	if (ext == "mp3")
	{
		return "audio/mp3";
	}

	return nullptr;
}

StaticFileManager::StaticFileManager(const oatpp::String& path) : path_string_(path), path_(std::string(path)) {}

std::shared_ptr<StaticFileManager> StaticFileManager::createShared(const oatpp::String& path)
{
	return std::make_shared<StaticFileManager>(path);
}

oatpp::String StaticFileManager::getFile(const oatpp::String& filename, bool ignore_cache)
{
	std::lock_guard<oatpp::concurrency::SpinLock> lock(lock_);

	oatpp::String path = path_string_ + "/" + filename;
	
	if (ignore_cache)
	{
		return oatpp::String::loadFromFile(path->c_str());
	}
	
	auto& file = cache_[filename];

	if (file)
	{
		return file;
	}

	file = oatpp::String::loadFromFile(path->c_str());
	return file;
}

bool StaticFileManager::isFileExists(const std::string& filename)
{
	return std::experimental::filesystem::exists(path_ / filename);
}

std::vector<std::string> StaticFileManager::getRecordsNames(int entries)
{
	std::vector<std::string> result;

	for (const auto& entry : std::experimental::filesystem::directory_iterator(path_ / "playlists\\records"))
	{
		if (entry.path().has_extension())
		{
			continue;
		}

		result.push_back(entry.path().filename().string());

		if (result.size() == entries)
		{
			break;
		}
	}

	return result;
}