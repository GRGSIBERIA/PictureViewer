#pragma once
#include <Siv3D.hpp>
#include <functional>

class MemoryReader : public IReader
{
	const int64 size;
	void* buffer;
	int64 pos = 0;

public:
	MemoryReader(const int64 _size, const char* _buffer)
		: IReader(), size(_size), buffer(malloc(_size))
	{
		memcpy_s(buffer, size, _buffer, size);
	}

	virtual ~MemoryReader() {
		free(buffer);
	}

	bool isOpen() const override { return true; }

	int64 size() const override { return (int64)size; }

	int64 getPos() const override { return pos; }

	bool setPos(int64 newPos) override { 
		pos = newPos; 
		return true;
	}

	int64 skip(int64 offset) override {
		pos += offset;
		if (pos >= size)
		{
			pos = size - 1;
			return false;
		}
		return true;
	}

	int64 read(void* dest, int64 _size) override
	{
		void* offset = (void*)((int64)buffer + pos);
		if (pos + _size >= size)
		{
			int64 can_read = size - _size - pos;
			memcpy_s(dest, _size, offset, can_read);
			pos += can_read;
			return can_read;
		}	// このif文に入ったら、ここから先は実行されない
		else if (_size < 0)
		{
			return 0;
		}

		memcpy_s(dest, _size, offset, _size);
		pos += _size;
		return _size;
	}

	int64 read(void* dest, int64 _pos, int64 _size) override
	{
		if (_pos >= size)
		{
			pos = size;
			return 0;
		}
		else if (_pos < 0)
		{
			_pos = 0;
		}
		pos = _pos;

		return read(dest, _size);
	}

	int64 lookahead(void* _buffer, int64 _size) const
	{
		if (pos + _size >= size)
		{
			int64 can_read = size - _size - pos;
			memcpy_s(_buffer, _size, buffer, can_read);
			return can_read;
		}
		else if (_size < 0)
		{
			return 0;
		}
		memcpy_s(_buffer, _size, buffer, _size);
		return _size;
	}

	int64 lookahead(void* _buffer, int64 _pos, int64 _size) const
	{

	}
};