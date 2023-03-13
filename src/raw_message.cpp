#include <raw_message.hpp>

namespace gocator {

Message Message::from_stream(std::ifstream& file) {
    Message m;
    file.read(reinterpret_cast<char*>(&m.timestamp), sizeof(m.timestamp));
    file.read(reinterpret_cast<char*>(&m.frame_index), sizeof(m.frame_index));
    file.read(reinterpret_cast<char*>(&m.encoder), sizeof(m.encoder));

    file.read(reinterpret_cast<char*>(&m.x_res), sizeof(m.x_res));
    file.read(reinterpret_cast<char*>(&m.x_offset), sizeof(m.x_offset));
    file.read(reinterpret_cast<char*>(&m.z_res), sizeof(m.z_res));
    file.read(reinterpret_cast<char*>(&m.z_res), sizeof(m.z_res));

    size_t count{};
    file.read(reinterpret_cast<char*>(&count), sizeof(count));
    m.points.reserve(count);

    for (size_t i = 0; i < count; ++i) {
        m.points.emplace_back();
        auto& p = m.points.back();

        file.read(reinterpret_cast<char*>(&p.x), sizeof(p.x));
        file.read(reinterpret_cast<char*>(&p.z), sizeof(p.z));
        file.read(reinterpret_cast<char*>(&p.i), sizeof(p.i));
    }

    return m;
}

void Message::to_stream(std::ofstream& file) const {
    file.write(reinterpret_cast<const char*>(&timestamp), sizeof(timestamp));
    file.write(reinterpret_cast<const char*>(&frame_index), sizeof(frame_index));
    file.write(reinterpret_cast<const char*>(&encoder), sizeof(encoder));

    file.write(reinterpret_cast<const char*>(&x_res), sizeof(x_res));
    file.write(reinterpret_cast<const char*>(&x_offset), sizeof(x_offset));
    file.write(reinterpret_cast<const char*>(&z_res), sizeof(z_res));
    file.write(reinterpret_cast<const char*>(&z_res), sizeof(z_res));

    size_t count = points.size();
    file.write(reinterpret_cast<const char*>(&count), sizeof(count));
    for (const auto& p : points) {
        file.write(reinterpret_cast<const char*>(&p.x), sizeof(p.x));
        file.write(reinterpret_cast<const char*>(&p.z), sizeof(p.z));
        file.write(reinterpret_cast<const char*>(&p.i), sizeof(p.i));
    }
}
}  // namespace gocator