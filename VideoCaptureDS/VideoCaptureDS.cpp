/*----- PROTECTED REGION ID(VideoCaptureDS.cpp) ENABLED START -----*/
static const char *RcsId = "$Id:  $";
//=============================================================================
//
// file :        VideoCaptureDS.cpp
//
// description : C++ source for the VideoCaptureDS class and its commands.
//               The class is derived from Device. It represents the
//               CORBA servant object which will be accessed from the
//               network. All commands which can be executed on the
//               VideoCaptureDS are implemented in this file.
//
// project :     
//
// This file is part of Tango device class.
// 
// Tango is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Tango is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Tango.  If not, see <http://www.gnu.org/licenses/>.
// 
// $Author:  $
//
// $Revision:  $
// $Date:  $
//
// $HeadURL:  $
//
//=============================================================================
//                This file is generated by POGO
//        (Program Obviously used to Generate tango Object)
//=============================================================================


#include <VideoCaptureDS.h>
#include <VideoCaptureDSClass.h>

/*----- PROTECTED REGION END -----*/	//	VideoCaptureDS.cpp

/**
 *  VideoCaptureDS class description:
 *    
 */

//================================================================
//  The following table gives the correspondence
//  between command and method names.
//
//  Command name  |  Method name
//================================================================
//  State         |  Inherited (no method)
//  Status        |  Inherited (no method)
//  Capture       |  capture
//  Reconnect     |  reconnect
//================================================================

//================================================================
//  Attributes managed are:
//================================================================
//  Jpeg   |  Tango::DevEncoded	Scalar
//  Frame  |  Tango::DevUChar	Image  ( max = 3840 x 720)
//================================================================

namespace VideoCaptureDS_ns
{
/*----- PROTECTED REGION ID(VideoCaptureDS::namespace_starting) ENABLED START -----*/

//	static initializations

/*----- PROTECTED REGION END -----*/	//	VideoCaptureDS::namespace_starting

//--------------------------------------------------------
/**
 *	Method      : VideoCaptureDS::VideoCaptureDS()
 *	Description : Constructors for a Tango device
 *                implementing the classVideoCaptureDS
 */
//--------------------------------------------------------
VideoCaptureDS::VideoCaptureDS(Tango::DeviceClass *cl, string &s)
 : TANGO_BASE_CLASS(cl, s.c_str())
{
	/*----- PROTECTED REGION ID(VideoCaptureDS::constructor_1) ENABLED START -----*/
	init_device();
	
	/*----- PROTECTED REGION END -----*/	//	VideoCaptureDS::constructor_1
}
//--------------------------------------------------------
VideoCaptureDS::VideoCaptureDS(Tango::DeviceClass *cl, const char *s)
 : TANGO_BASE_CLASS(cl, s)
{
	/*----- PROTECTED REGION ID(VideoCaptureDS::constructor_2) ENABLED START -----*/
	init_device();
	
	/*----- PROTECTED REGION END -----*/	//	VideoCaptureDS::constructor_2
}
//--------------------------------------------------------
VideoCaptureDS::VideoCaptureDS(Tango::DeviceClass *cl, const char *s, const char *d)
 : TANGO_BASE_CLASS(cl, s, d)
{
	/*----- PROTECTED REGION ID(VideoCaptureDS::constructor_3) ENABLED START -----*/
	init_device();
	
	/*----- PROTECTED REGION END -----*/	//	VideoCaptureDS::constructor_3
}

//--------------------------------------------------------
/**
 *	Method      : VideoCaptureDS::delete_device()
 *	Description : will be called at device destruction or at init command
 */
//--------------------------------------------------------
void VideoCaptureDS::delete_device()
{
	DEBUG_STREAM << "VideoCaptureDS::delete_device() " << device_name << endl;
	/*----- PROTECTED REGION ID(VideoCaptureDS::delete_device) ENABLED START -----*/
	
	delete cv_cam;
	delete image_no_image;
	
	/*----- PROTECTED REGION END -----*/	//	VideoCaptureDS::delete_device
	delete[] attr_Jpeg_read;
	delete[] attr_Frame_read;
}

//--------------------------------------------------------
/**
 *	Method      : VideoCaptureDS::init_device()
 *	Description : will be called at device initialization.
 */
//--------------------------------------------------------
void VideoCaptureDS::init_device()
{
	DEBUG_STREAM << "VideoCaptureDS::init_device() create device " << device_name << endl;
	/*----- PROTECTED REGION ID(VideoCaptureDS::init_device_before) ENABLED START -----*/
	
	//	Initialization before get_device_property() call
	
	/*----- PROTECTED REGION END -----*/	//	VideoCaptureDS::init_device_before
	

	//	Get the device properties from database
	get_device_property();
	
	attr_Jpeg_read = new Tango::DevEncoded[1];
	attr_Frame_read = new Tango::DevUChar[3840*720];
	/*----- PROTECTED REGION ID(VideoCaptureDS::init_device) ENABLED START -----*/

	cv_cam = nullptr;
	image_no_image = nullptr;
	update_cv_cam();

	/*----- PROTECTED REGION END -----*/	//	VideoCaptureDS::init_device
}

//--------------------------------------------------------
/**
 *	Method      : VideoCaptureDS::get_device_property()
 *	Description : Read database to initialize property data members.
 */
//--------------------------------------------------------
void VideoCaptureDS::get_device_property()
{
	/*----- PROTECTED REGION ID(VideoCaptureDS::get_device_property_before) ENABLED START -----*/
	
	//	Initialize property data members
	
	/*----- PROTECTED REGION END -----*/	//	VideoCaptureDS::get_device_property_before


	//	Read device properties from database.
	Tango::DbData	dev_prop;
	dev_prop.push_back(Tango::DbDatum("Source"));
	dev_prop.push_back(Tango::DbDatum("Mode"));
	dev_prop.push_back(Tango::DbDatum("Height"));
	dev_prop.push_back(Tango::DbDatum("Width"));
	dev_prop.push_back(Tango::DbDatum("JpegQuality"));

	//	is there at least one property to be read ?
	if (dev_prop.size()>0)
	{
		//	Call database and extract values
		if (Tango::Util::instance()->_UseDb==true)
			get_db_device()->get_property(dev_prop);
	
		//	get instance on VideoCaptureDSClass to get class property
		Tango::DbDatum	def_prop, cl_prop;
		VideoCaptureDSClass	*ds_class =
			(static_cast<VideoCaptureDSClass *>(get_device_class()));
		int	i = -1;

		//	Try to initialize Source from class property
		cl_prop = ds_class->get_class_property(dev_prop[++i].name);
		if (cl_prop.is_empty()==false)	cl_prop  >>  source;
		else {
			//	Try to initialize Source from default device value
			def_prop = ds_class->get_default_device_property(dev_prop[i].name);
			if (def_prop.is_empty()==false)	def_prop  >>  source;
		}
		//	And try to extract Source value from database
		if (dev_prop[i].is_empty()==false)	dev_prop[i]  >>  source;

		//	Try to initialize Mode from class property
		cl_prop = ds_class->get_class_property(dev_prop[++i].name);
		if (cl_prop.is_empty()==false)	cl_prop  >>  mode;
		else {
			//	Try to initialize Mode from default device value
			def_prop = ds_class->get_default_device_property(dev_prop[i].name);
			if (def_prop.is_empty()==false)	def_prop  >>  mode;
		}
		//	And try to extract Mode value from database
		if (dev_prop[i].is_empty()==false)	dev_prop[i]  >>  mode;

		//	Try to initialize Height from class property
		cl_prop = ds_class->get_class_property(dev_prop[++i].name);
		if (cl_prop.is_empty()==false)	cl_prop  >>  height;
		else {
			//	Try to initialize Height from default device value
			def_prop = ds_class->get_default_device_property(dev_prop[i].name);
			if (def_prop.is_empty()==false)	def_prop  >>  height;
		}
		//	And try to extract Height value from database
		if (dev_prop[i].is_empty()==false)	dev_prop[i]  >>  height;

		//	Try to initialize Width from class property
		cl_prop = ds_class->get_class_property(dev_prop[++i].name);
		if (cl_prop.is_empty()==false)	cl_prop  >>  width;
		else {
			//	Try to initialize Width from default device value
			def_prop = ds_class->get_default_device_property(dev_prop[i].name);
			if (def_prop.is_empty()==false)	def_prop  >>  width;
		}
		//	And try to extract Width value from database
		if (dev_prop[i].is_empty()==false)	dev_prop[i]  >>  width;

		//	Try to initialize JpegQuality from class property
		cl_prop = ds_class->get_class_property(dev_prop[++i].name);
		if (cl_prop.is_empty()==false)	cl_prop  >>  jpegQuality;
		else {
			//	Try to initialize JpegQuality from default device value
			def_prop = ds_class->get_default_device_property(dev_prop[i].name);
			if (def_prop.is_empty()==false)	def_prop  >>  jpegQuality;
		}
		//	And try to extract JpegQuality value from database
		if (dev_prop[i].is_empty()==false)	dev_prop[i]  >>  jpegQuality;

	}

	/*----- PROTECTED REGION ID(VideoCaptureDS::get_device_property_after) ENABLED START -----*/
	
	//	Check device property data members init
	
	/*----- PROTECTED REGION END -----*/	//	VideoCaptureDS::get_device_property_after
}

//--------------------------------------------------------
/**
 *	Method      : VideoCaptureDS::always_executed_hook()
 *	Description : method always executed before any command is executed
 */
//--------------------------------------------------------
void VideoCaptureDS::always_executed_hook()
{
	DEBUG_STREAM << "VideoCaptureDS::always_executed_hook()  " << device_name << endl;
	/*----- PROTECTED REGION ID(VideoCaptureDS::always_executed_hook) ENABLED START -----*/
	
	//	code always executed before all requests
	
	/*----- PROTECTED REGION END -----*/	//	VideoCaptureDS::always_executed_hook
}

//--------------------------------------------------------
/**
 *	Method      : VideoCaptureDS::read_attr_hardware()
 *	Description : Hardware acquisition for attributes
 */
//--------------------------------------------------------
void VideoCaptureDS::read_attr_hardware(TANGO_UNUSED(vector<long> &attr_list))
{
	DEBUG_STREAM << "VideoCaptureDS::read_attr_hardware(vector<long> &attr_list) entering... " << endl;
	/*----- PROTECTED REGION ID(VideoCaptureDS::read_attr_hardware) ENABLED START -----*/
	
	//	Add your own code
	
	/*----- PROTECTED REGION END -----*/	//	VideoCaptureDS::read_attr_hardware
}

//--------------------------------------------------------
/**
 *	Read attribute Jpeg related method
 *	Description: 
 *
 *	Data type:	Tango::DevEncoded
 *	Attr type:	Scalar
 */
//--------------------------------------------------------
void VideoCaptureDS::read_Jpeg(Tango::Attribute &attr)
{
	DEBUG_STREAM << "VideoCaptureDS::read_Jpeg(Tango::Attribute &attr) entering... " << endl;
	/*----- PROTECTED REGION ID(VideoCaptureDS::read_Jpeg) ENABLED START -----*/
	//	Set the attribute value
	attr.set_value(&jpeg);
	
	/*----- PROTECTED REGION END -----*/	//	VideoCaptureDS::read_Jpeg
}
//--------------------------------------------------------
/**
 *	Read attribute Frame related method
 *	Description: 
 *
 *	Data type:	Tango::DevUChar
 *	Attr type:	Image max = 3840 x 720
 */
//--------------------------------------------------------
void VideoCaptureDS::read_Frame(Tango::Attribute &attr)
{
	DEBUG_STREAM << "VideoCaptureDS::read_Frame(Tango::Attribute &attr) entering... " << endl;
	/*----- PROTECTED REGION ID(VideoCaptureDS::read_Frame) ENABLED START -----*/
	//	Set the attribute value

	attr.set_value(attr_Frame_read, cam_mode == CameraMode::Grayscale ? width : width * 3, height);
	
	/*----- PROTECTED REGION END -----*/	//	VideoCaptureDS::read_Frame
}

//--------------------------------------------------------
/**
 *	Method      : VideoCaptureDS::add_dynamic_attributes()
 *	Description : Create the dynamic attributes if any
 *                for specified device.
 */
//--------------------------------------------------------
void VideoCaptureDS::add_dynamic_attributes()
{
	/*----- PROTECTED REGION ID(VideoCaptureDS::add_dynamic_attributes) ENABLED START -----*/
	
	//	Add your own code to create and add dynamic attributes if any
	
	/*----- PROTECTED REGION END -----*/	//	VideoCaptureDS::add_dynamic_attributes
}

//--------------------------------------------------------
/**
 *	Command Capture related method
 *	Description: 
 *
 */
//--------------------------------------------------------
void VideoCaptureDS::capture()
{
	DEBUG_STREAM << "VideoCaptureDS::Capture()  - " << device_name << endl;
	/*----- PROTECTED REGION ID(VideoCaptureDS::capture) ENABLED START -----*/

	cv::Mat* image_to_show;
	cv::Mat image_cam;

	if (get_state() == Tango::FAULT || !cv_cam || !cv_cam->isOpened())
	{
		image_to_show = image_no_image;
	}
	else
	{
		cv_cam->read(image_cam);

		image_to_show = image_cam.empty() ? image_no_image : &image_cam;
	}

	cv::Mat image_converted;
	cv::Mat image_to_jpeg;

	switch (cam_mode)
	{
	case CameraMode::RGB:
		cv::cvtColor(*image_to_show, image_converted, cv::COLOR_BGR2RGB);
		cv::cvtColor(*image_to_show, image_to_jpeg, cv::COLOR_BGR2RGBA);
		jpeg.encode_jpeg_rgb32(image_to_jpeg.data, width, height, std::max(1.0, std::min(100.0, double(jpegQuality))));
		image_to_show = &image_converted;
		break;
	case CameraMode::BGR:
		cv::cvtColor(*image_to_show, image_to_jpeg, cv::COLOR_BGR2BGRA);
		jpeg.encode_jpeg_rgb32(image_to_jpeg.data, width, height, std::max(1.0, std::min(100.0, double(jpegQuality))));
		break;
	case CameraMode::Grayscale:
		cv::cvtColor(*image_to_show, image_converted, cv::COLOR_BGR2GRAY);
		jpeg.encode_jpeg_gray8(image_converted.data, width, height, std::max(1.0, std::min(100.0, double(jpegQuality))));
		image_to_show = &image_converted;
		break;
	default:
		break;
	}

	attr_Jpeg_read->encoded_data.length(jpeg.get_size());
	std::memcpy(attr_Jpeg_read->encoded_data.NP_data(), jpeg.get_data(), jpeg.get_size());

	if (cam_mode != CameraMode::None)
	{
		int size = image_to_show->total() * image_to_show->elemSize() * sizeof(uchar);

		if (size <= 3840 * 720)
		{
			std::memcpy(attr_Frame_read, image_to_show->data, size);
			push_change_event("Frame", attr_Frame_read, cam_mode == CameraMode::Grayscale ? width : width * 3, height);
		}
	}

	push_change_event("Jpeg", attr_Jpeg_read, jpeg.get_size());
	/*----- PROTECTED REGION END -----*/	//	VideoCaptureDS::capture
}
//--------------------------------------------------------
/**
 *	Command Reconnect related method
 *	Description: 
 *
 */
//--------------------------------------------------------
void VideoCaptureDS::reconnect()
{
	DEBUG_STREAM << "VideoCaptureDS::Reconnect()  - " << device_name << endl;
	/*----- PROTECTED REGION ID(VideoCaptureDS::reconnect) ENABLED START -----*/

	update_cv_cam();

	/*----- PROTECTED REGION END -----*/	//	VideoCaptureDS::reconnect
}
//--------------------------------------------------------
/**
 *	Method      : VideoCaptureDS::add_dynamic_commands()
 *	Description : Create the dynamic commands if any
 *                for specified device.
 */
//--------------------------------------------------------
void VideoCaptureDS::add_dynamic_commands()
{
	/*----- PROTECTED REGION ID(VideoCaptureDS::add_dynamic_commands) ENABLED START -----*/
	
	//	Add your own code to create and add dynamic commands if any
	
	/*----- PROTECTED REGION END -----*/	//	VideoCaptureDS::add_dynamic_commands
}

/*----- PROTECTED REGION ID(VideoCaptureDS::namespace_ending) ENABLED START -----*/

void VideoCaptureDS::update_cv_cam()
{
	delete cv_cam;
	delete image_no_image;

	cv_cam = new cv::VideoCapture(source);

	width = width < 3840 ? width : 3840;
	height = height < 720 ? height : 720;

	if (!cv_cam->isOpened())
	{
		set_state(Tango::FAULT);
	}
	else
	{
		set_state(Tango::ON);

		cv_cam->set(cv::CAP_PROP_FRAME_HEIGHT, height);
		cv_cam->set(cv::CAP_PROP_FRAME_WIDTH, width);

		if (cv_cam->get(cv::CAP_PROP_FRAME_HEIGHT) != height || cv_cam->get(cv::CAP_PROP_FRAME_WIDTH) != width)
		{
			set_state(Tango::FAULT);
		}
	}

	image_no_image = new cv::Mat(200, 270, CV_8UC3, cv::Scalar(255, 0, 0));
	cv::putText(*image_no_image, "No image", cv::Point(width / 2 - 50, height / 2), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));

	if (get_state() == Tango::FAULT)
	{
		return;
	}

	if (mode == "RGB" || mode == "rgb")
	{
		cam_mode = CameraMode::RGB;
	}
	else if (mode == "BGR" || mode == "bgr")
	{
		cam_mode = CameraMode::BGR;
	}
	else if (mode == "Grayscale" || mode == "grayscale")
	{
		cam_mode = CameraMode::Grayscale;
	}
	else
	{
		cam_mode = CameraMode::None;
		set_state(Tango::FAULT);
	}
}

/*----- PROTECTED REGION END -----*/	//	VideoCaptureDS::namespace_ending
} //	namespace
