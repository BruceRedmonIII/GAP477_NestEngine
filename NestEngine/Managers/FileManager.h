#pragma once
#include "Manager.h"
#include "../Configuration/LogMacro.hint"
#include <map>
#include <concepts>
#include <filesystem>
template <class Type> concept PathType = std::is_convertible_v<Type, const std::filesystem::path&>;

namespace nest
{
	class FileManager
	{
		static FileManager* s_pFileManager;
		static inline std::multimap<HashedId, const std::filesystem::path> s_fileMap{};
		FileManager() = default;
	public:
		static FileManager* GetInstance();
		static void Destroy();

		// Adds paths with given key, uses Variadic for simplicity
		template <PathType ...Type>
		static void AddPaths(HashedId key, Type&&... args)
		{
			(AddPathInternal(key, args), ...);
		}
		// Attempts to find an existing file using the paths with the given key
		// returns false on failure
		// returns true on success and updates the file parameter to be the successful path found
		static std::string ResolvePath(HashedId key, const std::filesystem::path& file)
		{
			auto [first, last] = s_fileMap.equal_range(key);
			for (const auto& [_, value] : std::ranges::subrange(first, last))
			{
				std::filesystem::path path = value;
				path += file;
				if (std::filesystem::exists(path) == true)
				{
					return path.string();
				}
			}
			_LOG_V(LogType::kError, "File not found! - ", file);
			return file.string();
		}
	private:
		// Internal function used by AddPaths to iteratively add path to map
		static void AddPathInternal(HashedId key, const std::filesystem::path& path)
		{
			s_fileMap.emplace(key, path);
		}
	};
}