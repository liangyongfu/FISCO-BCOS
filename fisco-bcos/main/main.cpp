/**
 * @CopyRight:
 * FISCO-BCOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FISCO-BCOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FISCO-BCOS.  If not, see <http://www.gnu.org/licenses/>
 * (c) 2016-2018 fisco-dev contributors.
 *
 * @brief: empty framework for main of FISCO-BCOS
 *
 * @file: main.cpp
 * @author: yujiechen
 * @date 2018-08-24
 */
#include "ExitHandler.h"
#include <include/BuildInfo.h>
#include <libdevcore/easylog.h>
#include <libinitializer/Initializer.h>
#include <boost/program_options.hpp>
#include <clocale>
#include <ctime>
#include <iostream>
#include <memory>

//seekfunbook
#include <libdevcore/db.h>
#include <leveldb/db.h>
#include <leveldb/status.h>
#include <leveldb/write_batch.h>
#include <libdevcore/BasicLevelDB.h>
#include <thread>
#include <chrono>
#include <libdevcore/FixedHash.h>
//end seekfunbook

#if FISCO_EASYLOG
INITIALIZE_EASYLOGGINGPP
#endif

using namespace std;
using namespace dev::initializer;
using namespace dev;

void setDefaultOrCLocale()
{
#if __unix__
    if (!std::setlocale(LC_ALL, ""))
    {
        setenv("LC_ALL", "C", 1);
    }
#endif
}

void version()
{
    std::cout << "FISCO-BCOS Version : " << FISCO_BCOS_PROJECT_VERSION << std::endl;
    std::cout << "Build Time         : " << DEV_QUOTED(FISCO_BCOS_BUILD_TIME) << std::endl;
    std::cout << "Build Type         : " << DEV_QUOTED(FISCO_BCOS_BUILD_PLATFORM) << "/"
              << DEV_QUOTED(FISCO_BCOS_BUILD_TYPE) << std::endl;
    std::cout << "Git Branch         : " << DEV_QUOTED(FISCO_BCOS_BUILD_BRANCH) << std::endl;
    std::cout << "Git Commit Hash    : " << DEV_QUOTED(FISCO_BCOS_COMMIT_HASH) << std::endl;
}

std::string srcPath;
uint64_t start;
std::shared_ptr<LedgerManager> manager;

string initCommandLine(int argc, const char* argv[])
{
    boost::program_options::options_description main_options("Usage of FISCO-BCOS");
    main_options.add_options()("help,h", "print help information")
        ("version,v", "version of FISCO-BCOS")("config,c",boost::program_options::value<std::string>(), "config file path, eg. config.ini")
        ("src,s",boost::program_options::value<std::string>(),"src path")
        ("startBlock,t", boost::program_options::value<uint64_t>(),"startblock");
    boost::program_options::variables_map vm;
    try
    {
        boost::program_options::store(
            boost::program_options::parse_command_line(argc, argv, main_options), vm);
    }
    catch (...)
    {
        std::cout << "invalid parameters" << std::endl;
        std::cout << main_options << std::endl;
        exit(0);
    }
    /// help information
    if (vm.count("help") || vm.count("h"))
    {
        std::cout << main_options << std::endl;
        exit(0);
    }
    /// version information
    if (vm.count("version") || vm.count("v"))
    {
        version();
        exit(0);
    }
    string configPath("./config.ini");
    if (vm.count("config") || vm.count("c"))
    {
        configPath = vm["config"].as<std::string>();
    }
    else if (boost::filesystem::exists(configPath))
    {
        std::cout << "use default configPath : " << configPath << std::endl;
    }
    else
    {
        std::cout << main_options << std::endl;
        exit(0);
    }

    if(vm.count("src"))
    {
        srcPath = vm["src"].as<std::string>();
    }
    else{
        std::cout << "need src " << std::endl;
        exit(0);
    }

    if(vm.count("startBlock"))
    {
        start = vm["startBlock"].as<uint64_t>();
    }
    else{
        std::cout << "need startBlock " << std::endl;
        exit(0);
    }

    return configPath;
}


//seekfunbook
void toBigEndian_s(uint64_t _val, dev::bytesRef& o_out)
{
	static_assert(std::is_same<bigint, uint64_t>::value || !std::numeric_limits<uint64_t>::is_signed, "only unsigned types or bigint supported"); //bigint does not carry sign bit on shift
	for (auto i = o_out.size(); i != 0; _val >>= 8, i--)
	{
		uint64_t v = _val & (uint64_t)0xff;
		o_out[i - 1] = (typename dev::bytesRef::value_type)(uint8_t)v;
	}
}

//线程函数
void getSrcData()
{
    //获取参数
    std::string path("./config.ini");

    //打开数据库
    std::cout << " start open srd db" << std::endl;
    leveldb::Options option;
    leveldb::ReadOptions readoption;
    option.create_if_missing = true;
    option.max_open_files = 100;

    dev::db::BasicLevelDB* pleveldb = nullptr;
    leveldb::Status status = dev::db::BasicLevelDB::Open(option, "/home/seekfunbook/", &(pleveldb));

    if (!status.ok())
    {
        throw std::runtime_error(" src ::::open LevelDB failed");
        exit(0);
    }
    std::cout << "end  open srd db" << std::endl;

    for( ; 1==1; start++)
    {
        uint64_t oldblocknumber = manager->blockChain(1)->number();
        //获取已迁移之后的块
        std::string blockHash;
        std::cout << "start get block hash by number" << std::endl;
        {        
            static thread_local FixedHash<33> h;
            dev::bytesRef aa(h.data() + 24, 8);
            toBigEndian_s(start, aa);
            h[32] = (uint8_t)0;
            leveldb::Slice sl((char*)h.data());
            pleveldb->Get(readoption,sl,&blockHash);
            std::cout << "end get block hash by number:" << start << " hash:" <<blockHash << " slice:" << sl.data() << std::endl;
        }
        

        std::string d;
        std::cout << "start get  block" << std::endl;
        {
            static thread_local FixedHash<33> h2(RLP(blockHash).toHash<h256>());
            h2[32] = (uint8_t)0;
            leveldb::Slice s2((char*)h2.data());
            pleveldb->Get(readoption, s2, &d);
            if (d.empty())
            {
                std::cout << "get  block error" << "   block hash:" << blockHash<< std::endl;
                throw std::runtime_error(" src ::::get block failed");
                exit(0);
            }

        }  
        std::cout << "end get  block" << std::endl;

        //获取块中交易，并将交易发送到ledger的txpool中
        RLP block(d);        
        std::cout << "there are " << block[1].itemCount() << " txs in block " << start << std::endl;

        continue;

        for (unsigned i = 0; i < block[1].itemCount(); i++)
        {
            //block[1][i].data()  可以转为transaction
            Transaction tx(block[1][i].data(),CheckTransaction::None);
            manager->txPool(1)->submit(tx);            
        }   
    
        //块中交易发送完后，设置ledger 出块
        manager->setNewBlock(true);

        //循环等待块出完后 再发送下一个块的交易
        uint64_t newblocknum = 0;
        while(true)
        {
            newblocknum = manager->blockChain(1)->number();
            if(newblocknum > oldblocknumber)
            {
                break;
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
        std::cout << "src block:" << start << "   new block:" << newblocknum << std::endl;
        
    }

}

//end seekfunbook



int main(int argc, const char* argv[])
{
    /// set LC_ALL
    setDefaultOrCLocale();
    /// init params
    string configPath = initCommandLine(argc, argv);
    /// callback initializer to init all ledgers
    auto initialize = std::make_shared<Initializer>();
    try
    {
        std::cout << "Initializing..." << std::endl;
        initialize->init(configPath);
    }
    catch (std::exception& e)
    {
        std::cerr << "Init failed!!!" << std::endl;
        return -1;
    }
    version();
    // get datetime and output welcome info
    char buffer[40];
    auto currentTime = chrono::system_clock::to_time_t(chrono::system_clock::now());
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&currentTime));
    std::cout << "[" << buffer << "] ";
    std::cout << "The FISCO-BCOS is running..." << std::endl;
    ExitHandler exitHandler;
    signal(SIGABRT, &ExitHandler::exitHandler);
    signal(SIGTERM, &ExitHandler::exitHandler);
    signal(SIGINT, &ExitHandler::exitHandler);

    //seekfunbook   //启动一个新线程读取块，发送交易，一个块发送完之后 主动让出块，，，出块成功后再发下一个块的交易
    manager = initialize->ledgerInitializer()->ledgerManager();
    std::cout << "start a new thread." << std::endl;
    std::thread th(getSrcData);
    th.detach();
    std::cout << " end start a new thread." << std::endl;
    //end seekfunbook

    while (!exitHandler.shouldExit())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        LogInitializer::logRotateByTime();
    }
    initialize.reset();
    currentTime = chrono::system_clock::to_time_t(chrono::system_clock::now());
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&currentTime));
    std::cout << "[" << buffer << "] ";
    std::cout << "FISCO-BCOS program exit normally." << std::endl;
    return 0;
}
