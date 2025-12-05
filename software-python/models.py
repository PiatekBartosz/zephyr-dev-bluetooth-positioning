from pydantic import BaseModel, Field, field_validator
from typing import Dict


class Position(BaseModel):
    x: int = Field(..., ge=0)
    y: int = Field(..., ge=0)


class CanvasConfig(BaseModel):
    width: int = Field(..., ge=100)
    height: int = Field(..., ge=100)


class Config(BaseModel):
    beacons: Dict[str, Position]
    canvas: CanvasConfig

    @field_validator("beacons")
    def validate_mac_keys(cls, value):
        import re
        mac_regex = re.compile(r"^[0-9A-F]{2}(:[0-9A-F]{2}){5}$")

        for mac in value.keys():
            if not mac_regex.match(mac):
                raise ValueError(f"Invalid MAC address format: {mac}")

        return value
