#ifndef RTW_STB_IMAGE_H
#define RTW_STB_IMAGE_H

#include "stb_image.h"
#include <cstdlib>
#include <iostream>

#endif
class rtw_image {
	const int bytes_per_pixel = 3;
	float* fdata = nullptr; // Linear floating point pixel data ,这是读出来的数据
	unsigned char* bdata = nullptr; // Linear 8-bit pixel data，这是读出的数据转换后的unsigned char数据
	int image_width = 0; // Loaded image width
	int image_height = 0; // Loaded image height
	int bytes_per_scanline = 0;
	static unsigned char float_to_byte(float value) {
		if (value <= 0.0)
			return 0;
		if (1.0 <= value)
			return 255;
		return static_cast<unsigned char>(256.0 * value);
	}

	void convert_to_bytes() { //将0-1的float rgb转为unsigned int 1byte rgb
		int total_bytes = image_width * image_height * bytes_per_pixel;
		bdata = new unsigned char[total_bytes];

		auto *bptr = bdata;
		auto* fptr = fdata;
		for (int i = 0; i < total_bytes; i++, fptr++, bptr++)
			*bptr = float_to_byte(*fptr);

	}
	static int clamp(int x, int low, int high) {
		if (x < low)return low;
		if (x < high)return x;
		return high - 1;
	}

public:
	bool load(const std::string& filename) {
		auto n = bytes_per_pixel;
		fdata = stbi_loadf(filename.c_str(), &image_width, &image_height, &n, bytes_per_pixel);
		if (fdata == nullptr) return false;
		bytes_per_scanline = image_width * bytes_per_pixel;
		convert_to_bytes();
		return true;
	}
	rtw_image() {}
	rtw_image(const char* image_filename) {
		auto filename = std::string(image_filename);
		auto imagedir = getenv("RTW_IMAGES");
		if (imagedir && load(std::string(imagedir) + "/" + image_filename)) return;
		if (load(filename)) return;
		if (load("images/" + filename)) return;
		if (load("../images/" + filename)) return;
		if (load("../../images/" + filename)) return;
		if (load("../../../images/" + filename)) return;
		if (load("../../../../images/" + filename)) return;
		if (load("../../../../../images/" + filename)) return;
		if (load("../../../../../../images/" + filename)) return;
		std::cerr << "ERROR: Could not load image file '" << image_filename << "'.\n";
	}
	int width() const { return (fdata == nullptr) ? 0 : image_width; }
	int height() const { return (fdata == nullptr) ? 0 : image_height; }

	const unsigned char* pixel_data(int x, int y)const {
		static unsigned char magenta[] = { 255, 0, 255 };
		if (bdata == nullptr)return magenta;

		x = clamp(x, 0, image_width);
		y = clamp(y, 0, image_height);
		return bdata + y * bytes_per_scanline + x * bytes_per_pixel;
	}
};