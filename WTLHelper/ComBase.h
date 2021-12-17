#pragma once

template<typename... Interfaces>
struct Implements : Interfaces... {
	static_assert(sizeof...(Interfaces) > 0);
};

template<typename... Interfaces>
struct ComClass : Interfaces... {
	static_assert(sizeof...(Interfaces) > 0);

	HRESULT __stdcall QueryInterface(REFIID riid, void** ppv) override {
		if (ppv == nullptr)
			return E_POINTER;
		*ppv = QueryInterface<Interfaces...>(riid);
		if (*ppv == nullptr)
			return E_NOINTERFACE;
		AddRef();
		return S_OK;
	}

	ULONG __stdcall AddRef() override {
		return ++_ref;
	}

	ULONG __stdcall Release() override {
		auto count = --_ref;
		if (count == 0)
			delete this;
		return count;
	}

	template<typename I, typename... Rest>
	void* QueryInterface(REFIID riid) {
		static_assert(std::is_base_of_v<IUnknown, I>);
		if (__uuidof(I) == riid)
			return static_cast<I*>(this);
		return QueryInterface<Rest...>(riid);
	}

	template<int = 0>
	void* QueryInterface(REFIID riid) {
		return nullptr;
	}

	template<typename I>
	I* QueryInterface() {
		I* p{ nullptr };
		QueryInterface(__uuidof(I), reinterpret_cast<void**>(&p));
		return p;
	}

private:
	int _ref{ 1 };

};
