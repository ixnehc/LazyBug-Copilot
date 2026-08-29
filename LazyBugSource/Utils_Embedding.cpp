#include "stdh.h"
#include "Utils_Embedding.h"

#include <curl/curl.h>

namespace
{

struct EmbedApiResponse
{
	std::string data;
};

size_t EmbedWriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	size_t totalSize = size * nmemb;
	EmbedApiResponse* response = static_cast<EmbedApiResponse*>(userp);
	response->data.append(static_cast<char*>(contents), totalSize);
	return totalSize;
}

int EmbedProgressCallback(void* userp, double, double, double, double)
{
	const std::atomic<bool>* enable = static_cast<const std::atomic<bool>*>(userp);
	return enable == nullptr || enable->load() ? 0 : 1;
}

}

namespace Utils
{

bool CallEmbeddingApi(const EmbedModelParam& modelParam,
                      const std::vector<std::string>& texts,
                      std::vector<std::vector<float>>& outEmbeddings,
                      const std::atomic<bool>* enable)
{
	outEmbeddings.clear();

	if (texts.empty())
		return true;

	if (!modelParam.IsValid())
		return false;

	std::string embedEndpoint = modelParam._endpoint;
	if (!embedEndpoint.empty() && embedEndpoint.back() == '/')
		embedEndpoint.pop_back();
	if (embedEndpoint.size() < 11 || embedEndpoint.compare(embedEndpoint.size() - 11, 11, "/embeddings") != 0)
		embedEndpoint += "/embeddings";

	json requestJson;
	requestJson["model"] = modelParam._modelName;
	requestJson["input"] = texts;
	std::string requestBody = requestJson.dump();

	CURL* curl = curl_easy_init();
	if (!curl)
		return false;

	struct curl_slist* headers = nullptr;
	headers = curl_slist_append(headers, "Content-Type: application/json; charset=utf-8");
	std::string authHeader = "Authorization: Bearer " + modelParam._apiKey;
	headers = curl_slist_append(headers, authHeader.c_str());

	EmbedApiResponse response;
	curl_easy_setopt(curl, CURLOPT_URL, embedEndpoint.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, EmbedWriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
	if (modelParam._timeoutSeconds > 0)
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, modelParam._timeoutSeconds);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, EmbedProgressCallback);
	curl_easy_setopt(curl, CURLOPT_XFERINFODATA, enable);

	CURLcode res = curl_easy_perform(curl);
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	if (res != CURLE_OK)
		return false;

	try
	{
		auto respJson = json::parse(response.data);
		if (respJson.contains("data") && respJson["data"].is_array())
		{
			const auto& dataArr = respJson["data"];
			outEmbeddings.resize(dataArr.size());
			for (size_t i = 0; i < dataArr.size(); i++)
			{
				const auto& item = dataArr[i];
				if (item.contains("embedding") && item["embedding"].is_array())
				{
					const auto& emb = item["embedding"];
					outEmbeddings[i].reserve(emb.size());
					for (const auto& val : emb)
						outEmbeddings[i].push_back(val.get<float>());
				}
			}
			return true;
		}
	}
	catch (...) {}

	return false;
}

}

