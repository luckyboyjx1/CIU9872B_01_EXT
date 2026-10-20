.PHONY: clean All Project_Title Project_Build

All: Project_Title Project_Build

Project_Title:
	@echo "----------Building project:[ CIU9872B_01_V1.1_7816_DemoCOS_Base - BuildSet ]----------"

Project_Build:
	@make -r -f CIU9872B_01_V1.1_7816_DemoCOS_Base.mk -j 6 -C  ./ 


clean:
	@echo "----------Cleaning project:[ CIU9872B_01_V1.1_7816_DemoCOS_Base - BuildSet ]----------"

