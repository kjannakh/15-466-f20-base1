#include "load_save_png.hpp"
#include "read_write_chunk.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <stdexcept>
#include <fstream>
#include <algorithm>

void write_palette_and_tile(std::vector< glm::u8vec4 > & pixels, std::ostream *to, const char *palette, const char *tile0, const char *tile1, bool write_palette) {
	std::vector< uint8_t > bit0;
	std::vector< uint8_t > bit1;
	std::vector< glm::u8vec4 > pal;
	int color_count = 0;
	uint8_t idx0=0, idx1=0;

	//put colors from png into a palette and create tiles
	for (int p = 0; p < pixels.size(); p++) {
		glm::u8vec4 pixel = pixels[p];
		int idx = -1;
		for (int c = 0; c < pal.size(); c++) {
			glm::u8vec4 color = pal[c];
			if (pixel == color) {
				idx = c;
				break;
			}
		}
		if (idx == -1) {
			pal.push_back(pixel);
			idx = color_count;
			color_count++;
		}
		switch (idx) {
		case 0:
			break;
		case 1:
			idx0 += 1;
			break;
		case 2:
			idx1 += 1;
			break;
		case 3:
			idx0 += 1;
			idx1 += 1;
			break;
		default:
			break;
		}
		if ((p + 1) % 8 > 0) {
			idx0 <<= 1;
			idx1 <<= 1;
		}
		else {
			bit0.push_back(idx0);
			bit1.push_back(idx1);
			idx0 = 0;
			idx1 = 0;
		}
	}

	if (write_palette) write_chunk< glm::u8vec4 >(palette, pal, to);
	write_chunk< glm::uint8_t >(tile0, bit0, to);
	write_chunk< glm::uint8_t >(tile1, bit1, to);
}

std::vector< glm::u8vec4 > write_palette(std::vector< glm::u8vec4 >& pixels, std::ostream* to, const char* palette) {
	std::vector< glm::u8vec4 > pal;

	//put colors from png into a palette and create tiles
	for (int p = 0; p < pixels.size(); p++) {
		glm::u8vec4 pixel = pixels[p];
		int idx = -1;
		for (int c = 0; c < pal.size(); c++) {
			glm::u8vec4 color = pal[c];
			if (pixel == color) {
				idx = c;
				break;
			}
		}
		if (idx == -1) {
			pal.push_back(pixel);
		}
	}
	write_chunk< glm::u8vec4 >(palette, pal, to);
	return pal;
}

void write_tile(std::vector< glm::u8vec4 >& pixels, std::ostream* to, std::vector< glm::u8vec4 > pal, const char* tile0, const char* tile1) {
	std::vector< uint8_t > bit0;
	std::vector< uint8_t > bit1;
	int color_count = 0;
	uint8_t idx0 = 0, idx1 = 0, idx = -1;
	glm::u8vec4 color;
	glm::u8vec4 pixel;

	for (int p = 0; p < pixels.size(); p++) {
		pixel = pixels[p];
		idx = -1;
		for (int c = 0; c < pal.size(); c++) {
			color = pal[c];
			if (pixel == color) {
				idx = c;
				break;
			}
		}
		switch (idx) {
		case 0:
			break;
		case 1:
			idx0 |= 1;
			break;
		case 2:
			idx1 |= 1;
			break;
		case 3:
			idx0 |= 1;
			idx1 |= 1;
			break;
		default:
			break;
		}
		if ((p + 1) % 8 > 0) {
			idx0 <<= 1;
			idx1 <<= 1;
		}
		else {
			bit0.push_back(idx0);
			bit1.push_back(idx1);
			idx0 = 0;
			idx1 = 0;
		}
	}
	write_chunk< glm::uint8_t >(tile0, bit0, to);
	write_chunk< glm::uint8_t >(tile1, bit1, to);
}

int main(int argc, char** argv) {
	// try-catch block taken from Jim McCann's game1 base code for robustness
#ifdef _WIN32

	//when compiled on windows, unhandled exceptions don't have their message printed, which can make debugging simple issues difficult.
	try {
#endif
		//constructor code from http://www.cplusplus.com/reference/ostream/ostream/ostream/
		std::filebuf fb;
		fb.open("assets.txt", std::ios::out | std::ios::binary);
		std::ostream to(&fb);

		//build background sprites
		std::vector< glm::u8vec4 > pixels;
		glm::uvec2 size(0, 0);
		load_png("cobble4.png", &size, &pixels, LowerLeftOrigin);

		const char* palette_name = "pal0";

		std::vector< glm::u8vec4 > pal0 = write_palette(pixels, &to, palette_name);

		std::vector< glm::u8vec4 > subpixels;
		for (int i = 0; i < 8; i++) {
			for (int j = 7; j >= 0; j--) {
				subpixels.push_back(pixels[i * 16 + j]);
			}
		}
		write_tile(subpixels, &to, pal0, "bgt0", "bgt1");

		subpixels.clear();
		for (int i = 8; i < 16; i++) {
			for (int j = 7; j >= 0; j--) {
				subpixels.push_back(pixels[i * 16 + j]);
			}
		}
		write_tile(subpixels, &to, pal0, "bgt2", "bgt3");
		
		subpixels.clear();
		for (int i = 0; i < 8; i++) {
			for (int j = 15; j >= 8; j--) {
				subpixels.push_back(pixels[i * 16 + j]);
			}
		}
		write_tile(subpixels, &to, pal0, "bgt4", "bgt5");

		subpixels.clear();
		for (int i = 8; i < 16; i++) {
			for (int j = 15; j >= 8; j--) {
				subpixels.push_back(pixels[i * 16 + j]);
			}
		}
		write_tile(subpixels, &to, pal0, "bgt6", "bgt7");

		// Build player sprites
		// Facing up
		pixels.clear();
		subpixels.clear();
		load_png("playerN.png", &size, &pixels, LowerLeftOrigin);

		for (int i = 0; i < 8; i++) {
			for (int j = 7; j >= 0; j--) {
				subpixels.push_back(pixels[i * 8 + j]);
			}
		}

		std::vector< glm::u8vec4 > pal1 = write_palette(subpixels, &to, "pal1");
		write_tile(subpixels, &to, pal1, "pc00", "pc01");

		// Facing right
		pixels.clear();
		subpixels.clear();
		load_png("playerE.png", &size, &pixels, LowerLeftOrigin);

		for (int i = 0; i < 8; i++) {
			for (int j = 7; j >= 0; j--) {
				subpixels.push_back(pixels[i * 8 + j]);
			}
		}
		write_tile(subpixels, &to, pal1, "pc10", "pc11");

		// Facing down
		pixels.clear();
		subpixels.clear();
		load_png("playerS.png", &size, &pixels, LowerLeftOrigin);

		for (int i = 0; i < 8; i++) {
			for (int j = 7; j >= 0; j--) {
				subpixels.push_back(pixels[i * 8 + j]);
			}
		}
		write_tile(subpixels, &to, pal1, "pc20", "pc21");

		/// Facing left
		pixels.clear();
		subpixels.clear();
		load_png("playerW.png", &size, &pixels, LowerLeftOrigin);

		for (int i = 0; i < 8; i++) {
			for (int j = 7; j >= 0; j--) {
				subpixels.push_back(pixels[i * 8 + j]);
			}
		}
		write_tile(subpixels, &to, pal1, "pc30", "pc31");

		// Facing Up-right
		pixels.clear();
		subpixels.clear();
		load_png("playerNE.png", &size, &pixels, LowerLeftOrigin);

		for (int i = 0; i < 8; i++) {
			for (int j = 7; j >= 0; j--) {
				subpixels.push_back(pixels[i * 8 + j]);
			}
		}
		write_tile(subpixels, &to, pal1, "pc40", "pc41");

		// Facing down-right
		pixels.clear();
		subpixels.clear();
		load_png("playerSE.png", &size, &pixels, LowerLeftOrigin);

		for (int i = 0; i < 8; i++) {
			for (int j = 7; j >= 0; j--) {
				subpixels.push_back(pixels[i * 8 + j]);
			}
		}
		write_tile(subpixels, &to, pal1, "pc50", "pc51");

		// facing down-left
		pixels.clear();
		subpixels.clear();
		load_png("playerSW.png", &size, &pixels, LowerLeftOrigin);

		for (int i = 0; i < 8; i++) {
			for (int j = 7; j >= 0; j--) {
				subpixels.push_back(pixels[i * 8 + j]);
			}
		}
		write_tile(subpixels, &to, pal1, "pc60", "pc61");

		// facing up-left
		pixels.clear();
		subpixels.clear();
		load_png("playerNW.png", &size, &pixels, LowerLeftOrigin);

		for (int i = 0; i < 8; i++) {
			for (int j = 7; j >= 0; j--) {
				subpixels.push_back(pixels[i * 8 + j]);
			}
		}
		write_tile(subpixels, &to, pal1, "pc70", "pc71");

		// Build slime assets
		pixels.clear();
		subpixels.clear();
		load_png("slime.png", &size, &pixels, LowerLeftOrigin);

		for (int i = 0; i < 8; i++) {
			for (int j = 7; j >= 0; j--) {
				subpixels.push_back(pixels[i * 8 + j]);
			}
		}
		std::vector< glm::u8vec4 > pal2 = write_palette(subpixels, &to, "pal2");
		write_tile(subpixels, &to, pal2, "sl00", "sl01");

		pixels.clear();
		subpixels.clear();
		load_png("superslime.png", &size, &pixels, LowerLeftOrigin);

		for (int i = 0; i < 8; i++) {
			for (int j = 7; j >= 0; j--) {
				subpixels.push_back(pixels[i * 8 + j]);
			}
		}
		std::vector< glm::u8vec4 > pal5 = write_palette(subpixels, &to, "pal5");
		write_tile(subpixels, &to, pal5, "sl10", "sl11");

		// Build bullet assets
		pixels.clear();
		subpixels.clear();
		load_png("bullet.png", &size, &pixels, LowerLeftOrigin);

		for (int i = 0; i < 8; i++) {
			for (int j = 7; j >= 0; j--) {
				subpixels.push_back(pixels[i * 8 + j]);
			}
		}
		std::vector< glm::u8vec4 > pal3 = write_palette(subpixels, &to, "pal3");
		write_tile(subpixels, &to, pal3, "bu00", "bu01");

		// Build score digit assets
		pixels.clear();
		subpixels.clear();
		load_png("0.png", &size, &pixels, LowerLeftOrigin);

		for (int i = 0; i < 8; i++) {
			for (int j = 7; j >= 0; j--) {
				subpixels.push_back(pixels[i * 8 + j]);
			}
		}
		std::vector< glm::u8vec4 > pal4 = write_palette(subpixels, &to, "pal4");
		write_tile(subpixels, &to, pal4, "0000", "0001");

		pixels.clear();
		subpixels.clear();
		load_png("1.png", &size, &pixels, LowerLeftOrigin);

		for (int i = 0; i < 8; i++) {
			for (int j = 7; j >= 0; j--) {
				subpixels.push_back(pixels[i * 8 + j]);
			}
		}
		write_tile(subpixels, &to, pal4, "0100", "0101");

		pixels.clear();
		subpixels.clear();
		load_png("2.png", &size, &pixels, LowerLeftOrigin);

		for (int i = 0; i < 8; i++) {
			for (int j = 7; j >= 0; j--) {
				subpixels.push_back(pixels[i * 8 + j]);
			}
		}
		write_tile(subpixels, &to, pal4, "0200", "0201");

		pixels.clear();
		subpixels.clear();
		load_png("3.png", &size, &pixels, LowerLeftOrigin);

		for (int i = 0; i < 8; i++) {
			for (int j = 7; j >= 0; j--) {
				subpixels.push_back(pixels[i * 8 + j]);
			}
		}
		write_tile(subpixels, &to, pal4, "0300", "0301");

		pixels.clear();
		subpixels.clear();
		load_png("4.png", &size, &pixels, LowerLeftOrigin);

		for (int i = 0; i < 8; i++) {
			for (int j = 7; j >= 0; j--) {
				subpixels.push_back(pixels[i * 8 + j]);
			}
		}
		write_tile(subpixels, &to, pal4, "0400", "0401");

		pixels.clear();
		subpixels.clear();
		load_png("5.png", &size, &pixels, LowerLeftOrigin);

		for (int i = 0; i < 8; i++) {
			for (int j = 7; j >= 0; j--) {
				subpixels.push_back(pixels[i * 8 + j]);
			}
		}
		write_tile(subpixels, &to, pal4, "0500", "0501");

		pixels.clear();
		subpixels.clear();
		load_png("6.png", &size, &pixels, LowerLeftOrigin);

		for (int i = 0; i < 8; i++) {
			for (int j = 7; j >= 0; j--) {
				subpixels.push_back(pixels[i * 8 + j]);
			}
		}
		write_tile(subpixels, &to, pal4, "0600", "0601");

		pixels.clear();
		subpixels.clear();
		load_png("7.png", &size, &pixels, LowerLeftOrigin);

		for (int i = 0; i < 8; i++) {
			for (int j = 7; j >= 0; j--) {
				subpixels.push_back(pixels[i * 8 + j]);
			}
		}
		write_tile(subpixels, &to, pal4, "0700", "0701");

		pixels.clear();
		subpixels.clear();
		load_png("8.png", &size, &pixels, LowerLeftOrigin);

		for (int i = 0; i < 8; i++) {
			for (int j = 7; j >= 0; j--) {
				subpixels.push_back(pixels[i * 8 + j]);
			}
		}
		write_tile(subpixels, &to, pal4, "0800", "0801");

		pixels.clear();
		subpixels.clear();
		load_png("9.png", &size, &pixels, LowerLeftOrigin);

		for (int i = 0; i < 8; i++) {
			for (int j = 7; j >= 0; j--) {
				subpixels.push_back(pixels[i * 8 + j]);
			}
		}
		write_tile(subpixels, &to, pal4, "0900", "0901");

		fb.close();

		return 0;

	// try-catch block taken from Jim McCann's game1 base code for robustness
#ifdef _WIN32
	}
	catch (std::exception const& e) {
		std::cerr << "Unhandled exception:\n" << e.what() << std::endl;
		return 1;
	}
	catch (...) {
		std::cerr << "Unhandled exception (unknown type)." << std::endl;
		throw;
	}
#endif
}