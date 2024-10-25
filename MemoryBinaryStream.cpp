#include <iostream>

/// MemoryBinaryStream.h
#include <vector>
#include <algorithm>
#include <iostream>
#include <streambuf>
#include <cstring>

class MemoryBuffer : public std::streambuf
{
private:
    std::vector<uint8_t> m_DefaultBuffer;
    std::vector<uint8_t>& m_Buffer;
    static const size_t RESERVE_SIZE = 1024;
    size_t m_Resize;

    void expand_buffer(size_t additional_size)
    {
        size_t current_size = m_Buffer.size();
        size_t required_size = current_size + additional_size;
        size_t new_capacity = ((required_size / m_Resize) + 1) * m_Resize;
        m_Buffer.reserve(new_capacity);
        setp((char*)m_Buffer.data(), (char*)m_Buffer.data() + m_Buffer.capacity());
        pbump(m_Buffer.size());
    }

public:
    void reserve(size_t newCapacity)
    {
        m_Buffer.reserve(newCapacity);
        setp((char*)m_Buffer.data(), (char*)m_Buffer.data() + m_Buffer.capacity());
        pbump(m_Buffer.size());
    }

    MemoryBuffer(size_t inResize = RESERVE_SIZE) : m_Buffer(m_DefaultBuffer), m_Resize(inResize) 
    {
        m_Buffer.reserve(m_Resize); 
        setp((char*)m_Buffer.data(), (char*)m_Buffer.data() + m_Buffer.capacity());
    }

    MemoryBuffer(std::vector<uint8_t>& inBuffer, size_t inResize = RESERVE_SIZE) : m_Buffer(inBuffer), m_Resize(inResize) 
    {
        setp((char*)m_Buffer.data(), (char*)m_Buffer.data() + m_Buffer.capacity());
    }

    const std::vector<uint8_t>& data() const { return m_Buffer; }

protected:
    virtual int_type overflow(int_type ch) override
    {
        if (ch != traits_type::eof())
        {
            expand_buffer(1);            
            m_Buffer.push_back(ch);
            pbump(1);
            return ch;
        }
        return traits_type::eof();
    }

    virtual std::streamsize xsputn(const char* s, std::streamsize n) override
    {
        if (epptr() - pptr() < n)
        {
            expand_buffer(n);
        }

        m_Buffer.resize(m_Buffer.size() + n);// Keep the number of element correct, no re-allocation occurs
        memcpy(pptr(), s, n);// Copy the values
        pbump(n);

        return n;
    }

    virtual int sync() override
    {
        // Values are already stored in the buffer, using resize 
        // to keep the number of element correct will erase them
        // For this reason we have to insert them again
        auto* lastPos = (char*)m_Buffer.data() + m_Buffer.size();
        m_Buffer.insert(m_Buffer.end(), lastPos, pptr());
        return 0;
    }

    virtual pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which) override
    {
        if (which & std::ios_base::out)
        {
            char* new_ptr = nullptr;
            switch (dir)
            {
            case std::ios_base::beg:
                new_ptr = (char*)m_Buffer.data() + off;
                break;
            case std::ios_base::cur:
                new_ptr = pptr() + off;
                break;
            case std::ios_base::end:
                new_ptr = (char*)m_Buffer.data() + m_Buffer.size() + off;
                break;
            }
            if (new_ptr >= (char*)m_Buffer.data() && new_ptr <= (char*)m_Buffer.data() + m_Buffer.size())
            {
                setp((char*)m_Buffer.data(), (char*)m_Buffer.data() + m_Buffer.capacity());
                pbump(new_ptr - (char*)m_Buffer.data());
                return new_ptr - (char*)m_Buffer.data();
            }
        }
        return pos_type(off_type(-1));
    }

    virtual pos_type seekpos(pos_type pos, std::ios_base::openmode which) override { return seekoff(pos, std::ios_base::beg, which); }
};

class MemoryBinaryStream : public std::iostream
{
private:
    MemoryBuffer m_Buffer;

public:
    MemoryBinaryStream() : std::iostream(&m_Buffer) {}
    MemoryBinaryStream(size_t inResize) : m_Buffer(inResize), std::iostream(&m_Buffer) {}
    MemoryBinaryStream(std::vector<uint8_t>& inBuffer) : m_Buffer(inBuffer), std::iostream(&m_Buffer) {}
    MemoryBinaryStream(std::vector<uint8_t>& inBuffer, size_t inResize) : m_Buffer(inBuffer, inResize), std::iostream(&m_Buffer) {}

    template <typename T>
    MemoryBinaryStream& operator<<(T value)
    {
        write((char*)&value, sizeof(T));
        return *this;
    }

    MemoryBinaryStream& operator<<(const char* pValue)
    {
        write((char*)pValue, strlen(pValue));
        return *this;
    }

    void reserve(size_t wide) 
    {
        m_Buffer.reserve(wide);
    }

    const std::vector<uint8_t>& data() const { return m_Buffer.data(); }
};
/// End

int main()
{
   MemoryBinaryStream memoryStream;
   memoryStream << "Hello World\n"; 
   memoryStream << 123u << 125u << (char)123 << (char)125<<"\n"; 
   auto data = memoryStream.data();
   for(const auto& d: data)
   {
       std::cout << d << " -> "<< (unsigned int)d <<std::endl ;
   }

   return 0;
}
