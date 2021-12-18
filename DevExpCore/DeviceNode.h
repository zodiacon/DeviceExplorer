#pragma once

class DeviceNode {
public:
	DeviceNode(DEVINST inst) : m_Inst(inst) {}

	operator DEVINST() const {
		return m_Inst;
	}

	static std::vector<DeviceNode> GetChildDevNodes(DeviceNode const& inst);
	static std::vector<DeviceNode> GetSiblingDevNodes(DeviceNode const& inst);

	std::vector<DeviceNode> GetChildren() const;
	std::vector<DeviceNode> GetSiblings() const;

private:
	DEVINST m_Inst;
};

