#include "bayan.h"

#include <boost/container_hash/hash.hpp>
#include <boost/filesystem.hpp>
#include <iostream>

namespace fs = boost::filesystem;

Bayan::FSigLoader::FSigLoader(FSig &fsig, size_t bs, Bayan::buf_t &buf) :
  m_fsig(fsig), m_bs(bs), m_buf(buf)
{
}

Bayan::hash_t
Bayan::FSigLoader::getHash(unsigned blocki)
{
  if(blocki < m_fsig.m_hash.size()) return m_fsig.m_hash[blocki];
  if(!m_ifs.is_open())
  {
    std::ifstream fin(m_fsig.m_path, std::ios_base::in|std::ios::binary);
    if(!fin.is_open()) throw std::runtime_error("can not open file for reading");
    m_ifs = std::move(fin);
    if(blocki != 0) m_ifs.seekg((std::streamoff)m_bs * blocki);
  }
  m_ifs.read(m_buf.data(), (std::streamsize)m_bs);
  if(m_ifs.bad()) throw std::runtime_error("error reading file " + m_fsig.m_path);
  if(m_bs*(blocki+1) > m_fsig.m_size)
  {
    for(unsigned kk = m_fsig.m_size % m_bs; kk < m_bs;++kk) {
      m_buf[kk] = 0;
    }
  }
  auto h = boost::hash<buf_t>()(m_buf);
  m_fsig.m_hash.emplace_back(h);
  return h;
}

void
Bayan::processFile(fs::directory_entry &f2_de)
  {
    if(!fs::is_regular_file(f2_de)) return;
    if(!std::regex_match(f2_de.path().filename().string(), m_rx)) return;
    auto size = file_size(f2_de);
    if(size < m_minsize) return;
    std::cout << "Processing " << f2_de.path() << " " << size << " byte(s): ";
    FSig fs2(f2_de.path().string(), size);
    FSigLoader fse2(fs2, m_bs, m_buf);
    bool dup_found = false;
    for(auto && fs1 : m_files)
    {
      FSigLoader fse1(fs1, m_bs, m_buf);
      if(fse1.size() == fse2.size())
      {
        bool hash_mismatch = false;
        for(unsigned ii = 0; ii < ((fse1.size()-1)/m_bs+1); ++ii)
        {
          if(fse1.getHash(ii) != fse2.getHash(ii))
          {
            hash_mismatch = true;
            break;
          }
        }
        if(!hash_mismatch)
        {
          std::cout << "is a dup of " + fse1.path() + "!\n";
          dup_found = true;
          fse1.emplace_back_dup(fse2.path());
          break;
        }
      }
    }
    if(!dup_found) m_files.emplace_back(fse2.fsig());
  }
