# samplv1_lv2ui.pro
#
NAME = samplv1

TARGET = $${NAME}_lv2ui
TEMPLATE = lib
CONFIG += shared plugin
LIBS += -L.

include(src_lv2.pri)

HEADERS = \
	config.h \
	samplv1_lv2ui.h \
	samplv1widget_lv2.h

SOURCES = \
	samplv1_lv2ui.cpp \
	samplv1widget_lv2.cpp


unix {

	OBJECTS_DIR = .obj_lv2ui
	MOC_DIR     = .moc_lv2ui
	UI_DIR      = .ui_lv2ui

	isEmpty(PREFIX) {
		PREFIX = /usr/local
	}

	isEmpty(LIBDIR) {
		TARGET_ARCH = $$system(uname -m)
		contains(TARGET_ARCH, x86_64) {
			LIBDIR = $${PREFIX}/lib64
		} else {
			LIBDIR = $${PREFIX}/lib
		}
	}

	contains(PREFIX, $$system(echo $HOME)) {
		LV2DIR = $${PREFIX}/.lv2
	} else {
		LV2DIR = $${LIBDIR}/lv2
	}

	TARGET_LV2UI = $${NAME}.lv2/$${NAME}_ui

	!exists($${TARGET_LV2UI}.so) {
		system(touch $${TARGET_LV2UI}.so)
	}

	INSTALLS += target

	target.path  = $${LV2DIR}/$${NAME}.lv2
	target.files = $${TARGET_LV2UI}.so $${TARGET_LV2UI}.ttl

	QMAKE_POST_LINK += $${QMAKE_COPY} -vp $(TARGET) $${TARGET_LV2UI}.so

	QMAKE_CLEAN += $${TARGET_LV2UI}.so

	LIBS += -l$${NAME} -l$${NAME}_ui -L$${NAME}.lv2 -Wl,-rpath,$${LIBDIR}:$${LV2DIR}/$${NAME}.lv2
}

QT += widgets xml
