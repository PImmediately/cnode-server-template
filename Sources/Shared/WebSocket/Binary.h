#ifndef BINARY_H
#define BINARY_H

#include <cstdint>
#include <vector>
#include <stdexcept>

class Binary {
public:

	inline void SetBuffer(uint8_t* buffer, size_t length) {
		this->m_uBuffer.clear();
		this->m_uBuffer.insert(this->m_uBuffer.end(), buffer, (buffer + length));
	}

	inline uint8_t* GetBuffer() {
		uint8_t* buffer = new uint8_t[this->m_uBuffer.size()];
		std::memcpy(buffer, this->m_uBuffer.data(), this->m_uBuffer.size());
		return buffer;
	}

	inline size_t GetBufferLength() {
		return this->m_uBuffer.size();
	}

	inline void WriteUInt8(uint8_t v) {
		this->m_uBuffer.push_back(v);
	}

	inline void WriteVarUInt32(uint32_t v) {
		do {
			uint8_t byte = static_cast<uint8_t>(v & 0x7F); 
			v >>= 7; 
			if (v > 0) {
				byte |= 0x80;
			}
			this->WriteUInt8(byte);
		} while (v > 0);
	}

	inline void WriteVarInt32(int32_t v) {
		this->WriteVarUInt32((v << 1) ^ (v >> 31));
	}

	inline void WriteFloat32(float v) {
		uint8_t buffer[sizeof(float)];
		memcpy(buffer, &v, sizeof(float));
		this->m_uBuffer.insert(this->m_uBuffer.end(), buffer, (buffer + sizeof(float)));
	}

	inline void WriteFloat64(double v) {
		uint8_t buffer[sizeof(double)];
		memcpy(buffer, &v, sizeof(double));
		this-> m_uBuffer.insert(this->m_uBuffer.end(), buffer, (buffer + sizeof(double)));
	}

	inline void WriteString(const char* v) {
		size_t length = strlen(v) + 1;
		this->m_uBuffer.insert(this->m_uBuffer.end(), v, (v + length));
	}


	inline uint8_t ReadUInt8() {
		if (this->m_uBuffer.empty()) {
			throw std::out_of_range("Buffer underflow: Cannot read 'ReadUInt8'.");
		}

		uint8_t v = this->m_uBuffer.front();
		this->m_uBuffer.erase(this->m_uBuffer.begin());
		return v;
	}

	inline uint32_t ReadVarUInt32() {
		uint32_t v = 0;
		for (int i = 0; i < 64; i += 7) {
			uint8_t byte = this->ReadUInt8();
			v |= (byte & 0x7F) << i;
			if (!(byte & 0x80)) {
				break;
			}
		}
		return v;
	}

	inline int32_t ReadVarInt32() {
		uint32_t raw = this->ReadVarUInt32();
		return (raw >> 1) ^ -(raw & 1);
	}

	inline float ReadFloat32() {
		if (this->m_uBuffer.size() < sizeof(float)) {
			throw std::out_of_range("Buffer underflow: Cannot read 'ReadFloat32'.");
		}

		float v;
		memcpy(&v, this->m_uBuffer.data(), sizeof(float));
		this->m_uBuffer.erase(this->m_uBuffer.begin(), (this->m_uBuffer.begin() + sizeof(float)));
		return v;
	}

	inline double ReadFloat64() {
		if (this->m_uBuffer.size() < sizeof(double)) {
			throw std::out_of_range("Buffer underflow: Cannot read 'ReadFloat64'.");
		}

		double v;
		memcpy(&v, this->m_uBuffer.data(), sizeof(double));
		this->m_uBuffer.erase(this->m_uBuffer.begin(), (this->m_uBuffer.begin() + sizeof(double)));
		return v;
	}

	inline char* ReadString() {
		auto null_at = std::find(m_uBuffer.begin(), m_uBuffer.end(), NULL);
		if (null_at == m_uBuffer.end()) {
			throw std::out_of_range("Buffer underflow: Cannot read 'ReadString'.");
		}
		
		size_t length = std::distance(m_uBuffer.begin(), null_at) + 1;
		char* v = new char[length];
		memcpy(v, m_uBuffer.data(), length); 
		m_uBuffer.erase(m_uBuffer.begin(), m_uBuffer.begin() + length);
		return v;
	}

private:
	std::vector<uint8_t> m_uBuffer;

};

#endif // BINARY_H