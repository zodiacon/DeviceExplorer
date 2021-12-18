#include "pch.h"
#include "DeviceNode.h"

std::vector<DeviceNode> DeviceNode::GetChildDevNodes(DeviceNode const& inst) {
    DEVINST child;
    std::vector<DeviceNode> devnodes;
    devnodes.reserve(8);
    DeviceNode start(inst);
    while (CR_SUCCESS == ::CM_Get_Child(&child, start, 0)) {
        devnodes.push_back(child);
        start = child;
    }

    return devnodes;
}

std::vector<DeviceNode> DeviceNode::GetSiblingDevNodes(DeviceNode const& inst) {
    DEVINST sibling;
    std::vector<DeviceNode> devnodes;
    devnodes.reserve(8);
    DeviceNode start(inst);
    while (CR_SUCCESS == ::CM_Get_Sibling(&sibling, start, 0)) {
        devnodes.push_back(sibling);
        start = sibling;
    }

    return devnodes;
}

std::vector<DeviceNode> DeviceNode::GetChildren() const {
    return GetChildDevNodes(m_Inst);
}

std::vector<DeviceNode> DeviceNode::GetSiblings() const {
    return GetSiblingDevNodes(m_Inst);
}
