// Termm--Fall 2020

#include "Image.hpp"

#include <iostream>
#include <cstring>
#include <lodepng/lodepng.h>

const uint Image::m_colorComponents = 3; // Red, blue, green

//---------------------------------------------------------------------------------------
Image::Image()
  : m_width(0),
    m_height(0),
    m_data(0)
{
}

Image::Image(uint width, uint height,const char *filename) {
  m_height = height;
  m_width = width;

  size_t numElements = m_width * m_height * m_colorComponents;
	m_data = new double[numElements];
  memset(m_data, 0, numElements*sizeof(double));

  uint img_height, img_width;
  std::vector<unsigned char> img;
  unsigned int error = lodepng::decode(img, img_width, img_height, filename, LCT_RGB);
  if (error != 0) {    
    return;
  }
  // for (int i = 0; i < numElements ; ++i) {
    
  //   m_data[i] = img[i]/255.0f;
  // }
  for (int i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
      int y0 = (i * (img_height - 1)) / height;
      float ry = ((i * (img_height - 1)) % height) / (float) height;
      int x0 = (j * (img_width - 1)) / width;
      float rx = ((j * (img_width - 1)) % width) / (float) width;
      
      for (int k = 0; k < m_colorComponents; ++k) {
        size_t index = (y0*img_width + x0)*m_colorComponents + k;
        unsigned char C00 = img[index];
        unsigned char C10 = img[index + m_colorComponents];
        index = index + img_width*m_colorComponents;
        unsigned char C01 = img[index];
        unsigned char C11 = img[index + m_colorComponents];

        m_data[(i*width+j)*3+k] = (C00*(1 - ry)*(1 - rx) + C10*rx*(1 - ry) + C01*ry*(1 - rx) + C11*rx*ry)/255.0f;
      }
    }
  }
}

Image::Image(const char *filename) { 
	
  std::vector<unsigned char> img;
  unsigned int error = lodepng::decode(img, m_width, m_height, filename, LCT_RGB);
  if (error != 0) {    
    return;
  }

  size_t numElements = m_width * m_height * m_colorComponents;
	m_data = new double[numElements];
  for (int i = 0; i < numElements ; ++i) {
    m_data[i] = img[i]/255.0f;
  }  
}

//---------------------------------------------------------------------------------------
Image::Image(
		uint width,
		uint height
)
  : m_width(width),
    m_height(height)
{
	size_t numElements = m_width * m_height * m_colorComponents;
	m_data = new double[numElements];
	memset(m_data, 0, numElements*sizeof(double));
}

//---------------------------------------------------------------------------------------
Image::Image(const Image & other)
  : m_width(other.m_width),
    m_height(other.m_height),
    m_data(other.m_data ? new double[m_width * m_height * m_colorComponents] : 0)
{
  if (m_data) {
    std::memcpy(m_data, other.m_data,
                m_width * m_height * m_colorComponents * sizeof(double));
  }
}

//---------------------------------------------------------------------------------------
Image::~Image()
{
  delete [] m_data;
}

//---------------------------------------------------------------------------------------
Image & Image::operator=(const Image& other)
{
  delete [] m_data;
  
  m_width = other.m_width;
  m_height = other.m_height;
  m_data = (other.m_data ? new double[m_width * m_height * m_colorComponents] : 0);

  if (m_data) {
    std::memcpy(m_data,
                other.m_data,
                m_width * m_height * m_colorComponents * sizeof(double)
    );
  }
  
  return *this;
}

//---------------------------------------------------------------------------------------
uint Image::width() const
{
  return m_width;
}

//---------------------------------------------------------------------------------------
uint Image::height() const
{
  return m_height;
}

//---------------------------------------------------------------------------------------
double Image::operator()(uint x, uint y, uint i) const
{
  return m_data[m_colorComponents * (m_width * y + x) + i];
}

//---------------------------------------------------------------------------------------
double & Image::operator()(uint x, uint y, uint i)
{
  return m_data[m_colorComponents * (m_width * y + x) + i];
}

//---------------------------------------------------------------------------------------
static double clamp(double x, double a, double b)
{
	return x < a ? a : (x > b ? b : x);
}

//---------------------------------------------------------------------------------------
bool Image::savePng(const std::string & filename) const
{
	std::vector<unsigned char> image;

	image.resize(m_width * m_height * m_colorComponents);

	double color;
	for (uint y(0); y < m_height; y++) {
		for (uint x(0); x < m_width; x++) {
			for (uint i(0); i < m_colorComponents; ++i) {
				color = m_data[m_colorComponents * (m_width * y + x) + i];
				color = clamp(color, 0.0, 1.0);
				image[m_colorComponents * (m_width * y + x) + i] = (unsigned char)(255 * color);
			}
		}
	}

	// Encode the image
	unsigned error = lodepng::encode(filename, image, m_width, m_height, LCT_RGB);

	if(error) {
		std::cerr << "encoder error " << error << ": " << lodepng_error_text(error)
				<< std::endl;
	}

	return true;
}

//---------------------------------------------------------------------------------------
const double * Image::data() const
{
  return m_data;
}

//---------------------------------------------------------------------------------------
double * Image::data()
{
  return m_data;
}
