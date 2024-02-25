#include "bayan.h"
#include "hash32.h"

#include <boost/filesystem.hpp>
#include <iostream>

namespace fs = boost::filesystem;

Bayan::FSigLoader::FSigLoader(FSig &fsig, Bayan &bayan) :
  m_fsig(fsig), m_bayan(bayan)
{
}

Bayan::hash_t
Bayan::FSigLoader::getHash(unsigned blocki)
{
  auto bs = m_bayan.m_bs;
  auto& buf = m_bayan.m_buf;
  if(blocki < m_fsig.m_hash.size()) return m_fsig.m_hash[blocki];
  if(!m_ifs.is_open())
  {
    std::ifstream fin(m_fsig.m_path, std::ios_base::in|std::ios::binary);
    if(!fin.is_open()) throw std::runtime_error("can not open file for reading");
    m_ifs = std::move(fin);
    if(blocki != 0) m_ifs.seekg((std::streamoff)bs * blocki);
  }
  m_ifs.read(buf.data(), (std::streamsize)bs);
  if(m_ifs.bad()) throw std::runtime_error("error reading file " + m_fsig.m_path);
  if(bs*(blocki+1) > m_fsig.m_size)
  {
    for(unsigned kk = m_fsig.m_size % bs; kk < bs;++kk) {
      buf[kk] = 0;
    }
  }
  auto h = m_bayan.hash();
  m_fsig.m_hash.emplace_back(h);
  return h;
}

Bayan::Bayan(const boost::uintmax_t minsize, const size_t blocksize, std::regex && rx, hash32::hash_func_t &hf)
  : m_minsize(minsize), m_bs(blocksize), m_buf(m_bs), m_rx(rx), m_hf(hf)
{
}

Bayan::hash_t Bayan::hash() const
{
  return m_hf(m_buf);
}


void
Bayan::processFile(fs::directory_entry &f2_de)
{
  if(!fs::is_regular_file(f2_de)) return;
  if(!std::regex_match(f2_de.path().filename().string(), m_rx)) return;
  auto size = file_size(f2_de);
  if(size < m_minsize) return;
  FSig fs2(f2_de.path().string(), size);
  FSigLoader fsloader2(fs2, *this);
  bool dup_found = false;
  for(auto && fs1 : m_files)
  {
    FSigLoader fsloader1(fs1, *this);
    if(fsloader1.size() == fsloader2.size())
    {
      bool hash_mismatch = false;
      for(unsigned ii = 0; ii < ((fsloader1.size()-1)/m_bs+1); ++ii)
      {
        if(fsloader1.getHash(ii) != fsloader2.getHash(ii))
        {
          hash_mismatch = true;
          break;
        }
      }
      if(!hash_mismatch)
      {
        dup_found = true;
        fsloader1.emplace_back_dup(fsloader2.path());
        break;
      }
    }
  }
  if(!dup_found) m_files.emplace_back(fsloader2.fsig());
}

void Bayan::printDups() const
{
  for(auto && f : m_files)
  {
    if(!f.m_dups.empty())
    {
      std::cout << f.m_path << "\n";
      for(auto && fdup : f.m_dups)
      {
        std::cout << fdup << "\n";
      }
      std::cout << "\n";
    }
  }
}
