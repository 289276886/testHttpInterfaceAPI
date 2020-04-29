#include "MainWidget.h"
#include <QApplication>
#include <QTextCodec>


#include <stdio.h>
#include <iostream>
#include <string.h>
#include <curl/curl.h>
#include <json/json.h>

using namespace std;
#define MAX_BUF      65536
char wr_buf[MAX_BUF + 1];
int  wr_index;

int mainqt(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    MainWidget w;
    w.show();

    return a.exec();
}



wstring AsciiToUnicode(const string& str)
{
	// 预算-缓冲区中宽字节的长度  
	int unicodeLen = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, nullptr, 0);
	// 给指向缓冲区的指针变量分配内存  
	wchar_t *pUnicode = (wchar_t*)malloc(sizeof(wchar_t)*unicodeLen);
	// 开始向缓冲区转换字节  
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, pUnicode, unicodeLen);
	wstring ret_str = pUnicode;
	free(pUnicode);
	return ret_str;
}

string UnicodeToUtf8(const wstring& wstr)
{
	// 预算-缓冲区中多字节的长度  
	int ansiiLen = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
	// 给指向缓冲区的指针变量分配内存  
	char *pAssii = (char*)malloc(sizeof(char)*ansiiLen);
	// 开始向缓冲区转换字节  
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, pAssii, ansiiLen, nullptr, nullptr);
	string ret_str = pAssii;
	free(pAssii);
	return ret_str;
}


string AsciiToUtf8(const string& str)
{
	return UnicodeToUtf8(AsciiToUnicode(str));
}



int getSign(std::string str) 
{
	int hash = 202838;
	for (int i = 0; i < str.length(); i++) {
		hash ^= ((hash << 5) + str.at(i) + (hash >> 2));
	}
	return (hash & 0x7FFFFFFF);
}



size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
	int segsize = size * nmemb;
	if (wr_index + segsize > MAX_BUF) {
		*(int *)userp = 1;
		return 0;
	}

	memcpy((void *)&wr_buf[wr_index], buffer, (size_t)segsize);

	wr_index += segsize;

	wr_buf[wr_index] = 0;

	return segsize;

// 	string data((const char*)buffer, (size_t)size * nmemb);
// 
// 	*((std::stringstream*) userp) << data << endl;
// 
// 	return size * nmemb;

}
int main(void)
{
	CURL* curl = NULL;
	CURLcode res = CURLE_OK;
	//HTTP报文头
	struct curl_slist* headers = NULL;

	int  wr_error;
	wr_error = 0;
	wr_index = 0;

	//
	char szJsonData[1024];
	memset(szJsonData, 0, sizeof(szJsonData));
	std::string strJson = "'{";
	strJson += "\"user_name\" : \"test\",";
	strJson += "\"password\" : \"test123\"";
	strJson += "}'";
	strcpy(szJsonData, strJson.c_str());

	

	// 构造数组和字符串
	Json::Value root;
	Json::Value header;
	Json::Value body;
	//Json::Value sign;
	header["requestType"] = "systemLogin";
	header["loginUserId"] = "01a4131961a0ad5ad9ac3c9";
	header["macNo"] = "C8-1F-66-18-CA-07";
	header["appVersionCode"] = "1";
	header["appLang"] = "1";
	header["appChannel"] = "1";
	header["appDeviceType"] = "1";

	body["userId"] = "80ecd35572f64d8fb7e7c73b54fa1d33";

	root["body"] = body;
	root["header"] = header;
	
	
	
	Json::Reader reader;

	std::string strResult = root.toStyledString();
	Json::Value result;	

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();


	std::cout << root.toStyledString().c_str() << std::endl;
	//std::string strformat = root.toStyledString().c_str();
	//strformat = replace()
	std::string strpost = "jsonParams=";
	strpost += strResult.c_str();
	
	int nsign = getSign(root.toStyledString().c_str());

	char buf[100];
	itoa(nsign, buf, 10);
	std::string strsign = buf;

  	strpost += "&sign=";
 	strpost += strsign.c_str();

	if (curl) {
		// 构建HTTP报文头
		curl_slist *http_headers = NULL;
 		//http_headers = curl_slist_append(http_headers, "Accept: application/json");
 		//http_headers = curl_slist_append(http_headers, "Content-Type: application/json");//text/html
		http_headers = curl_slist_append(http_headers, "application/x-www-form-urlencoded;charset = UTF-8");
		

		// 设置method为post
	    curl_easy_setopt(curl, CURLOPT_POST, 1);
		//curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
		// 设置post请求的url地址
		curl_easy_setopt(curl, CURLOPT_URL, "http://prod.cgpipe.com:8088/admin/h5/users/getUserById");
		// 设置HTTP头
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, http_headers);
		// 设置发送超时时间
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1);

		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		// 执行单条请求
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strpost.c_str());
		std::cout << strpost.c_str() << std::endl;
		//curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, value.toStyledString().size());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		res = curl_easy_perform(curl);
		printf("res = %d (write_error = %d) ", res, wr_error);
	 	if (res == 0) printf("%s ", wr_buf);
		if (res != CURLE_OK) {
			
			std::cout << "curl_easy_perform() failed:" << curl_easy_strerror(res);
		}
		curl_slist_free_all(headers);
		
		curl_easy_cleanup(curl);
		
		curl_global_cleanup();
		return 0;
	}

	

	
	


	//设置http请求json格式参数
// 	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strpost.c_str()/*root.toStyledString().c_str()*/);
// 	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&wr_error);
// 	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
// 
// 	ret = curl_easy_perform(curl);
// 	printf("ret = %d (write_error = %d) ", ret, wr_error);
// 
// 	if (ret == 0) printf("%s ", wr_buf);
// 	curl_easy_cleanup(curl);
	return 0;
}