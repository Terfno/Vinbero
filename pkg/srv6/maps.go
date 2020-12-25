package srv6

import (
	"path"

	"github.com/takehaya/vinbero/pkg/xdptool"
)

const (
	STR_TXPort         = "tx_port"
	STR_TransitTablev4 = "transit_table_v4"
	STR_FunctionTable  = "function_table"
)

var (
	TXPortPath         = path.Join(xdptool.BpfFsPath, STR_TXPort)
	TransitTablev4Path = path.Join(xdptool.BpfFsPath, STR_TransitTablev4)
	FunctionTablePath  = path.Join(xdptool.BpfFsPath, STR_FunctionTable)
)
