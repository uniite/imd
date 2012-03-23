#
# Makefile for multifunction miscellaneous devices
#

obj-$(CONFIG_MCP)		+= mcp-core.o
obj-$(CONFIG_MCP_SA11X0)	+= mcp-sa11x0.o
obj-$(CONFIG_MCP_UCB1200)	+= ucb1x00-core.o
obj-$(CONFIG_MCP_UCB1200_TS)	+= ucb1x00-ts.o

ifeq ($(CONFIG_SA1100_ASSABET),y)
obj-$(CONFIG_MCP_UCB1200)	+= ucb1x00-assabet.o
endif

obj-$(CONFIG_TOUCHSCREEN_UCB1400)	+= mcp-ac97.o ucb1x00-core.o ucb1x00-ts.o ucb1400-adc.o

