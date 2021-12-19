#include "pch.h"
#include "DeviceNode.h"

std::vector<DeviceNode> DeviceNode::GetChildDevNodes(DeviceNode const& inst) {
    DEVINST child;
    std::vector<DeviceNode> devnodes;
    devnodes.reserve(8);
    DeviceNode start(inst);
    if (CR_SUCCESS == ::CM_Get_Child(&child, start, 0)) {
        devnodes.push_back(child);
        auto siblings = GetSiblingDevNodes(child);
        devnodes.insert(devnodes.end(), siblings.begin(), siblings.end());
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

std::vector<DEVPROPKEY> DeviceNode::GetPropertyKeys() const {
    std::vector<DEVPROPKEY> keys;
    ULONG count = 0;
    ::CM_Get_DevNode_Property_Keys(m_Inst, nullptr, &count, 0);
    if (count) {
        keys.resize(count);
        ::CM_Get_DevNode_Property_Keys(m_Inst, keys.data(), &count, 0);
    }
    return keys;
}

DeviceNodeStatus DeviceNode::GetStatus(DeviceNodeProblem* pProblem) const {
    ULONG status, problem;
    if (CR_SUCCESS == ::CM_Get_DevNode_Status(&status, &problem, m_Inst, 0)) {
        if (pProblem)
            *pProblem = static_cast<DeviceNodeProblem>(problem);
        return static_cast<DeviceNodeStatus>(status);
    }
    return DeviceNodeStatus::None;
}