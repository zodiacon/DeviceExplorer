#include "pch.h"
#include "DeviceNode.h"
#include <devpkey.h>

std::wstring DeviceNode::GetName() const {
	return GetProperty<std::wstring>(DEVPKEY_NAME);
}

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

bool DeviceNode::Enable() {
	return ::CM_Enable_DevInst(m_Inst, 0) == CR_SUCCESS;
}

bool DeviceNode::Disable() {
	return ::CM_Disable_DevNode(m_Inst, 0) == CR_SUCCESS;
}

bool DeviceNode::Uninstall() {
	return ::CM_Uninstall_DevNode(m_Inst, 0) == CR_SUCCESS;
}

bool DeviceNode::Rescan() {
	return ::CM_Reenumerate_DevInst(m_Inst, CM_REENUMERATE_NORMAL) == CR_SUCCESS;
}

bool DeviceNode::IsEnabled() const {
	return (GetStatus() & DeviceNodeStatus::Started) == DeviceNodeStatus::Started;
}

DEVPROPTYPE DeviceNode::GetPropertyType(DEVPROPKEY const& key) const {
	DEVPROPTYPE type;
	ULONG size = 0;
	::CM_Get_DevNode_Property(m_Inst, &key, &type, nullptr, &size, 0);
	return type;
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

std::vector<DeviceResource> DeviceNode::GetResources(LogicalConfigurationType type) {
	LOG_CONF log = 0;
	std::vector<DeviceResource> resources;
	if (CR_SUCCESS != ::CM_Get_First_Log_Conf(&log, m_Inst, static_cast<ULONG>(type)))
		return resources;

	resources.reserve(4);
	do {
		RES_DES rd = 0;
		RESOURCEID r = ResType_All;
		while (CR_SUCCESS == ::CM_Get_Next_Res_Des(&rd, rd ? rd : log, ResType_All, &r, 0)) {
			ULONG size = 0;
			::CM_Get_Res_Des_Data_Size(&size, rd, 0);
			auto buffer = std::make_unique<BYTE[]>(size);
			if (CR_SUCCESS == ::CM_Get_Res_Des_Data(rd, buffer.get(), size, 0)) {
				DeviceResource res;
				res.Type = (ResourceType)r;
				res._buffer = std::move(buffer);
				res._size = size;
				resources.push_back(std::move(res));
			}
		}
	} while (::CM_Get_Next_Log_Conf(&log, log, 0) == CR_SUCCESS);

	CM_Free_Log_Conf(log, 0);

	return resources;
}

std::unique_ptr<BYTE[]> DeviceNode::GetPropertyValue(DEVPROPKEY const& key, DEVPROPTYPE& type, ULONG* len) const {
	ULONG size = 0;
	::CM_Get_DevNode_Property(m_Inst, &key, &type, nullptr, &size, 0);
	auto value = std::make_unique<BYTE[]>(size);
	::CM_Get_DevNode_Property(m_Inst, &key, &type, value.get(), &size, 0);
	if (len)
		*len = size;
	return value;
}

ULONG DeviceNode::GetDepth() const {
	ULONG depth = -1;
	::CM_Get_Depth(&depth, m_Inst, 0);
	return depth;
}
