#include "pssp_program_settings.hpp"

//-----------------------------------------------------------------------------
// Begin pssp namespace
//-----------------------------------------------------------------------------
namespace pssp
{
void ProgramSettings::create_new_project(std::filesystem::path new_base_dir)
{
    // Always the same named file, in the direc tory
    std::ofstream file{new_base_dir / md_file, std::ios::binary};
    // Going to use a packer to make it easier to serialize the data
    // and write it out to the file
    msgpack::packer<std::ofstream> packer(file);
    packer.pack(window_settings);
    packer.pack(new_base_dir.string());
    // Keep it clean, close your files!
    file.close();
    // We still need to create the appropriate subdirectories
}

void ProgramSettings::load_project(std::filesystem::path md_full_path)
{
    std::ifstream file{md_full_path, std::ios::binary};
    // We want to know how big the file is, so we go to the end
    // and see how many bytes that was
    file.seekg(0, std::ios::end);
    std::ifstream::pos_type file_size{file.tellg()};
    // Back to the beginning
    file.seekg(0, std::ios::beg);
    // Going to use an unpacker to unpack the data
    msgpack::unpacker unpacker{};
    // Set aside the memory for the entire file
    unpacker.reserve_buffer(file_size);
    // Read into the unpacker's buffer
    file.read(unpacker.buffer(), file_size);
    file.close();
    // Mark how much of the buffer is full (should be all)
    unpacker.buffer_consumed(file_size);

    // Going to use object_handle to store unpacked object
    // and then convert to the correct type
    // First the window_settings struct
    msgpack::object_handle oh{};
    unpacker.next(oh);
    oh.get().convert(window_settings);
    // Then the path of the project
    unpacker.next(oh);
    // Doesn't handle std::filesystem::path type, need to do string then path
    std::string_view tmp{};
    oh.get().convert(tmp);
    base_dir = std::filesystem::path(tmp);
}
};
//-----------------------------------------------------------------------------
// End pssp namespace
//-----------------------------------------------------------------------------
