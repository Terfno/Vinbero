package xdptool

import (
	"fmt"
	"os"

	"github.com/newtools/ebpf"
	"github.com/pkg/errors"
	"github.com/vishvananda/netlink"
)

func LoadElf(filepath string) (*ebpf.Collection, error) {
	f, err := os.Open(filepath)
	if err != nil {
		return nil, errors.WithStack(err)
	}

	// Read ELF
	spec, err := ebpf.LoadCollectionSpecFromReader(f)
	if err != nil {
		return nil, errors.WithStack(err)
	}

	coll, err := ebpf.NewCollection(spec)
	if err != nil {
		return nil, errors.WithStack(err)
	}
	return coll, nil
}

func Attach(coll *ebpf.Collection, attach_func string, device string) error {

	prog, ok := coll.Programs[attach_func]
	if !ok {
		fmt.Println()
		return errors.New(fmt.Sprintf("%s not found in object", attach_func))
	}
	link, err := netlink.LinkByName(device)
	if err != nil {
		return errors.New(fmt.Sprintf("%s not found in object", device))
	}
	if err := netlink.LinkSetXdpFd(link, prog.FD()); err != nil {
		return errors.WithStack(err)
	}
	return nil
}

func Detach(device string) error {
	link, err := netlink.LinkByName(device)
	if err != nil {
		return errors.New(fmt.Sprintf("failed to get device %s: %s\n", device, err))
	}
	if err := netlink.LinkSetXdpFd(link, -1); err != nil {
		return errors.WithStack(err)
	}
	return nil
}
