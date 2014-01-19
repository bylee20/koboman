#include "barcode.hpp"

bool BarcodeScannerImage::convertToY8() {
	switch (m_format) {
	case Y8:
		return true;
	case Rgb32: {
		m_format = Y8;
		const int len = m_data.size()/4;
		auto dst = m_data.data();
		auto src = m_data.data();
		for (int i=0; i<len; ++i, src += 4)
			*dst++ = qGray(src[0], src[1], src[2]);
		m_bytes = len;
		return true;
	} case Gray32: {
		return false;
//		image.m_size = m_size;
//		image.m_format = Y8;
//		image.m_data.resize(m_data.size()/4);
//		auto dst = image.m_data.data();
//		auto src = this->m_data.data();
//		for (int i=0; i<image.m_data.size(); ++i, src += 4)
//			*dst++ = src[1];
//		break;
	} default:
		return false;
	}
	return true;
}

bool BarcodeScannerImage::getY8(BarcodeScannerImage &image) const {
	image.m_format = Invalid;
	switch (m_format) {
	case Y8:
		image = *this;
		break;
	case Rgb32: {
		image.m_size = m_size;
		image.m_format = Y8;
		image.m_data.resize(m_data.size()/4);
		auto dst = image.m_data.data();
		auto src = this->m_data.data();
		for (int i=0; i<image.m_data.size(); ++i, src += 4)
			*dst++ = qGray(src[0], src[1], src[2]);
		break;
	} case Gray32: {
		image.m_size = m_size;
		image.m_format = Y8;
		image.m_data.resize(m_data.size()/4);
		auto dst = image.m_data.data();
		auto src = this->m_data.data();
		for (int i=0; i<image.m_data.size(); ++i, src += 4)
			*dst++ = src[1];
		break;
	} default:
		return false;
	}
	return true;
}
