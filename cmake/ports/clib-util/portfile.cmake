# header-only library
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO SeaSparrowOG/CLibUtil
    REF 65c5b4411c761049c3bf14bee54500b81844fa49
    SHA512 4312accbd8a7e1dc0f6224225171ee36cb33c7f829daf5b8ba9bd59806d494e40e5673a9a582d11a29ac9570e8df298aa18be728ae26a9d9d63d291d6716955d
    HEAD_REF master
)

# Install codes
set(CLIBUTIL_SOURCE	${SOURCE_PATH}/include/ClibUtil)
file(INSTALL ${CLIBUTIL_SOURCE} DESTINATION ${CURRENT_PACKAGES_DIR}/include)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")