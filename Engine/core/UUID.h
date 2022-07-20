#pragma once

#include <cstdint>

namespace Engine {
	class UUID {
	public:
		UUID();
		UUID(uint64_t _uuid);
		UUID(const UUID&) = default;

		operator uint64_t() const { return uuid; }
	private:
		uint64_t uuid;
	};

}

namespace std {
	template <typename T> struct hash;

	template<>
	struct hash<Engine::UUID> {
		std::size_t operator()(const Engine::UUID& uuid) const {
			return (uint64_t)uuid;
		}
	};

}