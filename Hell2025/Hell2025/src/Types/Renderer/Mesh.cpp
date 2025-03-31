#include "Mesh.h"

void Mesh::SetName(const std::string& name) {
    m_name = name;
}

int32_t Mesh::GetIndexCount() {
    return indexCount;
}

const std::string& Mesh::GetName() {
    return m_name;
}
