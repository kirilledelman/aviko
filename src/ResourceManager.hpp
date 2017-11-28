#ifndef ResourceManager_hpp
#define ResourceManager_hpp

#include "common.h"

/* MARK:	-				Resource Manager
 
 Templated class for easy caching / finding different types of resources.
 Resource managers are member fields in Application class.
 
 Resource is loaded and returned by calling manager->Get( key )
 Which in turn calls a constructor of resource templated class with key
 if resource can't be loaded, Get returns a resource with .error set.
 
 Actual resource loading is implemented in subclasses of Resource class.
 -------------------------------------------------------------------- */


typedef enum {
	ERROR_NONE = 0,
	ERROR_NOT_FOUND,
	ERROR_COMPILE
} ResourceError;

// parent class to loadable resources
class Resource {
protected:
	
	int useCount = 0;
	
public:
	
	/// key in resource hash
	string key;
	
	/// true if failed to load
	ResourceError error = ERROR_NONE;
	
	/// set to true to prevent GC from unloading it
	bool dontUnload = false;
	
	/// use to tell GC when it's ok to unload this resource
	virtual void AdjustUseCount( int increment ) {
		this->useCount += increment;
		// printf( "%s.useCount = %d\n", key.c_str(), this->useCount );
	}
	
	/// returns true if this resource can be unloaded
	virtual bool CanUnload() {
		return ( !dontUnload && useCount <= 0 );
	}
	
	Resource() {}
	Resource( const char* ckey ){}
	~Resource() {}
	
	/// static function to split string via token
	static vector<string> splitString(const string& str, const string& delim ) {
		vector<string> tokens;
		size_t prev = 0, pos = 0;
		do {
			pos = str.find(delim, prev);
			if (pos == string::npos) pos = str.length();
			string token = str.substr(prev, pos-prev);
			if (!token.empty()) tokens.push_back(token);
			prev = pos + delim.length();
		}
		while (pos < str.length() && prev < str.length());
		return tokens;
	}
	
	/// static function to join string with token
	static string concatStrings(const vector<string> &elements, const std::string &separator, int maxElements=-1 ) {
		if (!elements.empty()) {
			stringstream ss;
			auto it = elements.cbegin();
			if ( maxElements < 0 ) maxElements = (int) elements.size();
			int nth = 0;
			while ( nth++ < maxElements ) {
				ss << *it++;
				if (it != elements.cend() && nth < maxElements )
					ss << separator;
				else
					return ss.str();
			}
		}
		return "";
	}
};

/// resource manager keeps a hash table by key of all loaded
/// resources of specific type - ImageResource, ChunkResource, etc.
template <class RESOURCE_TYPE> class ResourceManager {
public:
	
	unordered_map<string, RESOURCE_TYPE*> map;
	
public:
	
	/// get resource by filename/key
	RESOURCE_TYPE* Get( const char* ckey, bool incrementUseCount=false ){
		
		// find it
		string key = ckey;
		auto it = this->map.find( key );
		
		// if loaded return it
		if ( it != this->map.end() ) {
			return (RESOURCE_TYPE*) it->second;
		}
		
		// otherwise, load
		RESOURCE_TYPE* resource = new RESOURCE_TYPE( ckey );
		
		// add to map
		this->map.insert( make_pair( key, resource ) );
		
		// use count
		if ( incrementUseCount ) resource->AdjustUseCount( 1 );
		
		// return
		return resource;
		
	}
	
	// unload unused resources
	void UnloadUnusedResources() {
		auto it = this->map.begin();
		while( it != this->map.end() ) {
			if ( it->second->CanUnload() ) {
				delete it->second;
				it = this->map.erase( it );
			}
			else it++;
		}
	}
	
	// init
	ResourceManager() {}
	
	// destroy
	~ResourceManager() {

		// Since resource manager is a static instance, we're not going to release resources manually.
		// OS will do it on exit
		
		// release all resources
		for ( auto it = this->map.begin(); it != this->map.end(); ++it ) {
			// printf( "Unloading %s\n", it->first.c_str() );
			RESOURCE_TYPE* resource = it->second;
			delete resource;
		}
		this->map.clear();
		
	}
	
};

#endif /* ResourceManager_hpp */
