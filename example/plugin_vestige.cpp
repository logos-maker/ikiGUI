// COMPILE WITH...
// g++ plugin_vestige.cpp -o plugin_vestige.so -D__cdecl="" -fPIC -shared

#include "vestige.h"
#include <stdio.h>
#include <cstdlib>       // *********************************************************************************************************************************************
#include "../libs/ikigui.h" // *********************************************************************************************************************************************
//#include "gfx/bg.h"	// Embedded graphics for background in 32bit AARRGGBB BMP format.
#define VstInt32 int
#define VstIntPtr int64_t
#define kVstMaxVendorStrLen 16
#define effFlagsCanDoubleReplacing 4096
typedef audioMasterCallback VSTHostCallback;
typedef AEffect VSTPlugin;
ikigui_image bg;	// Raw global source graphics holder for background
struct ERect{
	int16_t top;    // Set to zero - Specifies the y-coordinate of the upper-left corner of the rectangle.
	int16_t left;   // Set to zero - Specifies the x-coordinate of the upper-left corner of the rectangle.
	int16_t bottom; // Set to window width - Specifies the x-coordinate of the lower-right corner of the rectangle.
	int16_t right;  // Set to window higth - Specifies the y-coordinate of the lower-right corner of the rectangle.
};
// Since the host is expecting a very specific API we need to make sure it has C linkage (not C++)
extern "C" {
	extern VSTPlugin *VSTPluginMain(VSTHostCallback vstHostCallback); // This is the main entry point to the VST plugin.
}

const VstInt32 PLUGIN_VERSION = 1000; //  Constant for the version of the plugin. For example 1100 for version 1.1.0.0

struct ERect myrect = {
    .top = 0,	 // Set to 0, anything else is futile.
    .left = 0,	 // Set to 0, anything else is futile.
    .bottom = 90,// Editor hight
    .right = 345,// Editor width
};

class VSTPluginWrapper // Encapsulates the plugin as a C++ class.so that it can be accessed when the host calls the plugin back (for example in `processDoubleReplacing`).
{
public:
  VSTPluginWrapper(VSTHostCallback vstHostCallback,
                   VstInt32 vendorUniqueID,
                   VstInt32 vendorVersion,
                   VstInt32 numParams,
                   VstInt32 numPrograms,
                   VstInt32 numInputs,
                   VstInt32 numOutputs);

  ~VSTPluginWrapper();

  inline VSTPlugin *getVSTPlugin()
  {
    return &_vstPlugin;
  }

  inline VstInt32 getNumInputs() const
  {
    return _vstPlugin.numInputs;
  }

  inline VstInt32 getNumOutputs() const
  {
    return _vstPlugin.numOutputs;
  }

  ikigui_window mywin ;

private:

  VSTHostCallback _vstHostCallback;  // the host callback (a function pointer)
  VSTPlugin _vstPlugin;  // the actual structure required by the host
  
};


// Callbacks: Host -> Plugin - For the host to call the plugin

void VSTPluginProcessSamplesFloat32(VSTPlugin *vstPlugin, float **inputs, float **outputs, VstInt32 sampleFrames)
{
  // we can get a hold to our C++ class since we stored it in the `object` field (see constructor)
  VSTPluginWrapper *wrapper = static_cast<VSTPluginWrapper *>(vstPlugin->object);

  // code speaks for itself: for each input (2 when stereo input),  after multiplying by 0.5 (which result in a 3dB attenuation of the sound)
  for(int i = 0; i < wrapper->getNumInputs(); i++) // iterating over every sample and writing the result in the outputs array.
  {
    auto inputSamples = inputs[i];
    auto outputSamples = outputs[i];
    for(int j = 0; j < sampleFrames; j++)
    {
      outputSamples[j] = inputSamples[j] * 0.5f;
    }
  }
}

// This is the callback that will be called to process the samples in the case of double precision. This is where the
void VSTPluginProcessSamplesFloat64(VSTPlugin *vstPlugin, double **inputs, double **outputs, VstInt32 sampleFrames)
{
  // we can get a hold to our C++ class since we stored it in the `object` field (see constructor)
  VSTPluginWrapper *wrapper = static_cast<VSTPluginWrapper *>(vstPlugin->object);

  // code speaks for itself: for each input (2 when stereo input), iterating over every sample and writing the
  // result in the outputs array after multiplying by 0.5 (which result in a 3dB attenuation of the sound)
  for(int i = 0; i < wrapper->getNumInputs(); i++)
  {
    auto inputSamples = inputs[i];
    auto outputSamples = outputs[i];
    for(int j = 0; j < sampleFrames; j++)
    {
      outputSamples[j] = inputSamples[j] * 0.5;
    }
  }
}

// This is the plugin called by the host to communicate with the plugin, mainly to request information (like the
VstIntPtr VSTPluginDispatcher(VSTPlugin *vstPlugin, VstInt32 opCode, VstInt32 index, VstIntPtr value, void *ptr, float opt)
{

  printf("called VSTPluginDispatcher(%d)\n", opCode);

  VstIntPtr v = 0;

  // we can get a hold to our C++ class since we stored it in the `object` field (see constructor)
  VSTPluginWrapper *wrapper = static_cast<VSTPluginWrapper *>(vstPlugin->object);
  // see aeffect.h/AEffectOpcodes and aeffectx.h/AEffectXOpcodes for details on all of them
  switch(opCode)
  {
	case effEditGetRect:		*(struct ERect**)ptr = &myrect ;   return true;		break; ///< [ptr]: #ERect** receiving pointer to editor size  @see ERect @see AEffEditor::getRect
	case effEditOpen:
		ikigui_image_create(&bg, 345,90); // ikigui_bmp_include(&bg,bg_array);  // Load background graphics.
		ikigui_image_gradient(&bg, 0xFFFF0000,0xFF0000FF);
		ikigui_window_open_editor(&wrapper->mywin,ptr,345,90);			// Open the editor window in host.
	break; ///< [ptr]: system dependent Window pointer, e.g. HWND on Windows  @see AEffEditor::open
	case effEditClose:		break; ///< no arguments @see AEffEditor::close

	break;

	case effEditIdle:
		ikigui_draw_image(&wrapper->mywin.frame,&bg, 0, 0); // Draw gackground.
		ikigui_window_update(&wrapper->mywin);
	break;
    // request for the category of the plugin: in this case it is an effect since it is modifying the input (as opposed
    // to generating sound)
    case effGetPlugCategory:
      return kPlugCategEffect;

    // called by the host when the plugin was called... time to reclaim memory!
    case effClose:
      delete wrapper;
      break;

    // request for the vendor string (usually used in the UI for plugin grouping)
    case effGetVendorString:
      strncpy(static_cast<char *>(ptr), "DSC", kVstMaxVendorStrLen);
      v = 1;
      break;

    // request for the version
    case effGetVendorVersion:
      return PLUGIN_VERSION;

    // ignoring all other opcodes
    default:
      printf("Unknown opCode %d [ignored] \n", opCode);
      break;
  }

  return v;
}

// Used for parameter setting (not used by this plugin)
void VSTPluginSetParameter(VSTPlugin *vstPlugin, VstInt32 index, float parameter)
{
  printf("called VSTPluginSetParameter(%d, %f)\n", index, parameter);
  // we can get a hold to our C++ class since we stored it in the `object` field (see constructor)
  VSTPluginWrapper *wrapper = static_cast<VSTPluginWrapper *>(vstPlugin->object);
}

// Used for parameter (not used by this plugin)
float VSTPluginGetParameter(VSTPlugin *vstPlugin, VstInt32 index)
{
  printf("called VSTPluginGetParameter(%d)\n", index);
  // we can get a hold to our C++ class since we stored it in the `object` field (see constructor)
  VSTPluginWrapper *wrapper = static_cast<VSTPluginWrapper *>(vstPlugin->object);
  return 0;
}

// Main constructor for our C++ class
VSTPluginWrapper::VSTPluginWrapper(audioMasterCallback vstHostCallback,
                                   VstInt32 vendorUniqueID,
                                   VstInt32 vendorVersion,
                                   VstInt32 numParams,
                                   VstInt32 numPrograms,
                                   VstInt32 numInputs,
                                   VstInt32 numOutputs) :
  _vstHostCallback(vstHostCallback)
{
  // Make sure that the memory is properly initialized
  memset(&_vstPlugin, 0, sizeof(_vstPlugin));

  // this field must be set with this constant...
  _vstPlugin.magic = kEffectMagic;

  // storing this object into the VSTPlugin so that it can be retrieved when called back (see callbacks for use)
  _vstPlugin.object = this;

  // specifying that we handle both single and double precision (there are other flags see aeffect.h/VstAEffectFlags)
  _vstPlugin.flags = effFlagsCanReplacing | effFlagsCanDoubleReplacing | effFlagsHasEditor ;   // ------------------------------------------------------------------------------------------------------------------------

  // initializing the plugin with the various values
  _vstPlugin.uniqueID = vendorUniqueID;
  _vstPlugin.version = vendorVersion;
  _vstPlugin.numParams = numParams;
  _vstPlugin.numPrograms = numPrograms;
  _vstPlugin.numInputs = numInputs;
  _vstPlugin.numOutputs = numOutputs;

  // setting the callbacks to the previously defined functions
  _vstPlugin.dispatcher = VSTPluginDispatcher;
  _vstPlugin.getParameter = VSTPluginGetParameter;
  _vstPlugin.setParameter = VSTPluginSetParameter;
  _vstPlugin.processReplacing = VSTPluginProcessSamplesFloat32;
  _vstPlugin.processDoubleReplacing = VSTPluginProcessSamplesFloat64;

  ikigui_window mywin ;

}

// Destructor called when the plugin is closed (see VSTPluginDispatcher with effClose opCode).
VSTPluginWrapper::~VSTPluginWrapper()
{
}

// Implementation of the main entry point of the plugin
VSTPlugin *VSTPluginMain(VSTHostCallback vstHostCallback)
{
  printf("called VSTPluginMain... \n");

  // simply create our plugin C++ class
  VSTPluginWrapper *plugin =
    new VSTPluginWrapper(vstHostCallback,
                         CCONST('\0', '\0', '\0', '\0'),
                         PLUGIN_VERSION, // version
                         0,    // number of parameters
                         0,    // number of presets
                         2,    // 2 audio inputs
                         2);   // 2 audio outputs

  return plugin->getVSTPlugin();  // return the plugin per the contract of the API
}
